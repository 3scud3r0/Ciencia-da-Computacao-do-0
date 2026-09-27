'use strict';

const state = {
  curriculum: [],
  projects: [],
  visibleProjects: 36,
  level: 'all',
  source: 'all',
  category: 'all',
  projectQuery: ''
};

const $ = (selector) => document.querySelector(selector);
const $$ = (selector) => [...document.querySelectorAll(selector)];

async function loadData() {
  const [curriculumResponse, projectResponse] = await Promise.all([
    fetch('./data/curriculum.json'),
    fetch('./data/projects.json')
  ]);
  if (!curriculumResponse.ok || !projectResponse.ok) {
    throw new Error('Falha ao carregar os dados do site.');
  }
  state.curriculum = await curriculumResponse.json();
  const payload = await projectResponse.json();
  state.projects = payload.projects;

  $('#moduleCount').textContent = state.curriculum.length;
  $('#chapterCount').textContent = state.curriculum.reduce((sum, m) => sum + m.chapters.length, 0);
  $('#projectCount').textContent = payload.counts.total;

  renderSidebar();
  renderCurriculum();
  populateCategories();
  renderProjects();
  updateProgress();
}

function renderSidebar() {
  $('#sidebarModules').innerHTML = state.curriculum.map((module) =>
    '<a href="#module-' + module.id + '"><b>' + module.id + '</b> · ' + escapeHtml(module.title) + '</a>'
  ).join('');
}

function renderCurriculum() {
  const filtered = state.curriculum.filter((module) =>
    state.level === 'all' || module.level === state.level
  );

  $('#curriculumGrid').innerHTML = filtered.map((module) => {
    const done = localStorage.getItem('cc0:' + module.id) === 'done';
    const sampleChapters = module.chapters.slice(0, 4).map((chapter) => '<li>' + escapeHtml(chapter) + '</li>').join('');
    return '<article id="module-' + module.id + '" class="curriculum-card">' +
      '<div class="top"><span class="module-num">MÓDULO ' + module.id + '</span><span class="level">' + escapeHtml(module.level) + '</span></div>' +
      '<h3>' + escapeHtml(module.title) + '</h3>' +
      '<p>' + module.chapters.length + ' capítulos · ' + escapeHtml(module.hours) + '</p>' +
      '<ul>' + sampleChapters + '</ul>' +
      '<div class="card-footer"><small>' + escapeHtml(module.projects.join(' · ')) + '</small>' +
      '<button class="complete-toggle ' + (done ? 'done' : '') + '" data-module="' + module.id + '">' + (done ? 'Concluído ✓' : 'Marcar concluído') + '</button></div>' +
      '</article>';
  }).join('');

  $$('.complete-toggle').forEach((button) => button.addEventListener('click', () => {
    const key = 'cc0:' + button.dataset.module;
    const done = localStorage.getItem(key) === 'done';
    localStorage.setItem(key, done ? '' : 'done');
    renderCurriculum();
    updateProgress();
  }));
}

function populateCategories() {
  const categories = [...new Set(state.projects.map((p) => p.category))].sort((a,b) => a.localeCompare(b));
  $('#categoryFilter').innerHTML = '<option value="all">Todas as categorias</option>' +
    categories.map((category) => '<option value="' + escapeAttribute(category) + '">' + escapeHtml(category) + '</option>').join('');
}

function filteredProjects() {
  const q = state.projectQuery.trim().toLowerCase();
  return state.projects.filter((project) => {
    const haystack = (project.title + ' ' + project.category + ' ' + project.source).toLowerCase();
    return (state.source === 'all' || project.source === state.source) &&
      (state.category === 'all' || project.category === state.category) &&
      (!q || haystack.includes(q));
  });
}

function renderProjects() {
  const filtered = filteredProjects();
  const visible = filtered.slice(0, state.visibleProjects);
  $('#projectMeta').textContent = filtered.length + ' projetos encontrados · exibindo ' + visible.length;
  $('#projectGrid').innerHTML = visible.map((project) =>
    '<a class="project-card" href="' + escapeAttribute(project.url) + '" target="_blank" rel="noreferrer">' +
      '<span class="source">' + escapeHtml(project.source) + '</span>' +
      '<h3>' + escapeHtml(project.title) + '</h3>' +
      '<p>' + escapeHtml(project.category) + ' ↗</p>' +
    '</a>'
  ).join('');
  $('#loadMore').style.display = visible.length < filtered.length ? 'flex' : 'none';
}

function updateProgress() {
  const done = state.curriculum.filter((module) => localStorage.getItem('cc0:' + module.id) === 'done').length;
  const percent = state.curriculum.length ? Math.round((done / state.curriculum.length) * 100) : 0;
  $('#progressText').textContent = percent + '% concluído · ' + done + '/' + state.curriculum.length + ' módulos';
  $('#progressBar').style.width = percent + '%';
}

function escapeHtml(value) {
  return String(value).replace(/[&<>"']/g, (char) => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#039;'}[char]));
}
function escapeAttribute(value) { return escapeHtml(value); }

$('#levelFilters').addEventListener('click', (event) => {
  const button = event.target.closest('[data-level]');
  if (!button) return;
  state.level = button.dataset.level;
  $$('#levelFilters .filter').forEach((item) => item.classList.toggle('active', item === button));
  renderCurriculum();
});

$('#projectSearch').addEventListener('input', (event) => {
  state.projectQuery = event.target.value;
  state.visibleProjects = 36;
  renderProjects();
});
$('#sourceFilter').addEventListener('change', (event) => {
  state.source = event.target.value;
  state.visibleProjects = 36;
  renderProjects();
});
$('#categoryFilter').addEventListener('change', (event) => {
  state.category = event.target.value;
  state.visibleProjects = 36;
  renderProjects();
});
$('#loadMore').addEventListener('click', () => {
  state.visibleProjects += 36;
  renderProjects();
});
$('#globalSearch').addEventListener('input', (event) => {
  const q = event.target.value.trim();
  if (!q) return;
  state.projectQuery = q;
  $('#projectSearch').value = q;
  location.hash = '#projetos';
  renderProjects();
});

loadData().catch((error) => {
  $('#projectMeta').textContent = error.message;
  console.error(error);
});
