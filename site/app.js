'use strict';

const state = {
  curriculum: [], projects: [], lessons: [], vendors: [],
  visibleProjects: 36, visibleLessons: 24, level: 'all', source: 'all', category: 'all', projectQuery: '', lessonQuery: '', lessonModule: 'all'
};
const $ = (selector) => document.querySelector(selector);
const $$ = (selector) => [...document.querySelectorAll(selector)];

async function loadData() {
  const responses = await Promise.all([
    fetch('./data/curriculum.json'),
    fetch('./data/projects.json'),
    fetch('./data/lessons.json'),
    fetch('./data/vendor-summary.json')
  ]);
  if (responses.some((response) => !response.ok)) throw new Error('Falha ao carregar os dados do site.');
  state.curriculum = await responses[0].json();
  const projectPayload = await responses[1].json();
  state.projects = projectPayload.projects;
  state.lessons = await responses[2].json();
  const vendorPayload = await responses[3].json();
  state.vendors = vendorPayload.sources;

  $('#moduleCount').textContent = state.curriculum.length;
  $('#chapterCount').textContent = state.curriculum.reduce((sum, m) => sum + m.chapters.length, 0);
  $('#projectCount').textContent = projectPayload.counts.total;

  renderSidebar(); populateLessonModules(); renderLessons(); renderCurriculum(); populateCategories(); renderProjects();
  renderVendors(vendorPayload.counts); updateProgress();
}

function renderSidebar() {
  $('#sidebarModules').innerHTML = state.curriculum.map((module) =>
    '<a href="#module-' + module.id + '"><b>' + module.id + '</b> · ' + escapeHtml(module.title) + '</a>'
  ).join('');
}

function populateLessonModules() {
  $('#lessonModuleFilter').innerHTML = '<option value="all">Todos os módulos</option>' +
    state.curriculum.map((module) =>
      '<option value="' + module.id + '">Módulo ' + module.id + ' · ' + escapeHtml(module.title) + '</option>'
    ).join('');
}

function filteredLessons() {
  const q = state.lessonQuery.trim().toLowerCase();
  return state.lessons.filter((lesson) => {
    const haystack = (lesson.title + ' ' + lesson.description + ' ' + lesson.moduleTitle).toLowerCase();
    return (state.lessonModule === 'all' || lesson.module === state.lessonModule) &&
      (!q || haystack.includes(q));
  });
}

function renderLessons() {
  const filtered = filteredLessons();
  const visible = filtered.slice(0, state.visibleLessons);
  $('#lessonMeta').textContent = filtered.length + ' aulas encontradas · exibindo ' + visible.length;
  $('#lessonGrid').innerHTML = visible.map((lesson) =>
    '<a class="project-card" href="' + escapeAttribute(lesson.url) + '" target="_blank" rel="noreferrer">' +
    '<span class="source">MÓDULO ' + escapeHtml(lesson.module) + ' · AUTORAL</span>' +
    '<h3>' + escapeHtml(lesson.title) + '</h3>' +
    '<p>' + escapeHtml(lesson.description) + ' ↗</p></a>'
  ).join('');
  $('#lessonLoadMore').style.display = visible.length < filtered.length ? 'flex' : 'none';
}

function renderCurriculum() {
  const filtered = state.curriculum.filter((module) => state.level === 'all' || module.level === state.level);
  $('#curriculumGrid').innerHTML = filtered.map((module) => {
    const done = localStorage.getItem('cc0:' + module.id) === 'done';
    const sample = module.chapters.slice(0,4).map((chapter) => '<li>' + escapeHtml(chapter) + '</li>').join('');
    return '<article id="module-' + module.id + '" class="curriculum-card">' +
      '<div class="top"><span class="module-num">MÓDULO ' + module.id + '</span><span class="level">' + escapeHtml(module.level) + '</span></div>' +
      '<h3>' + escapeHtml(module.title) + '</h3><p>' + module.chapters.length + ' capítulos · ' + escapeHtml(module.hours) + '</p>' +
      '<ul>' + sample + '</ul><div class="card-footer"><small>' + escapeHtml(module.projects.join(' · ')) + '</small>' +
      '<span><a class="module-open" href="https://github.com/3scud3r0/Ciencia-da-Computacao-do-0/tree/main/' + encodeURIComponent(module.slug) + '" target="_blank" rel="noreferrer">Abrir módulo ↗</a>' +
      '<button class="complete-toggle ' + (done ? 'done' : '') + '" data-module="' + module.id + '">' + (done ? 'Concluído ✓' : 'Concluir') + '</button></span></div></article>';
  }).join('');
  $$('.complete-toggle').forEach((button) => button.addEventListener('click', () => {
    const key = 'cc0:' + button.dataset.module;
    localStorage.setItem(key, localStorage.getItem(key) === 'done' ? '' : 'done');
    renderCurriculum(); updateProgress();
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
      (state.category === 'all' || project.category === state.category) && (!q || haystack.includes(q));
  });
}

function renderProjects() {
  const filtered = filteredProjects(), visible = filtered.slice(0,state.visibleProjects);
  $('#projectMeta').textContent = filtered.length + ' projetos encontrados · exibindo ' + visible.length;
  $('#projectGrid').innerHTML = visible.map((project) =>
    '<a class="project-card" href="' + escapeAttribute(project.url) + '" target="_blank" rel="noreferrer">' +
    '<span class="source">' + escapeHtml(project.source) + '</span><h3>' + escapeHtml(project.title) + '</h3>' +
    '<p>' + escapeHtml(project.category) + ' ↗</p></a>'
  ).join('');
  $('#loadMore').style.display = visible.length < filtered.length ? 'flex' : 'none';
}

function renderVendors(counts) {
  $('#vendorMeta').textContent = counts.total + ' snapshots no repositório · ' + counts.community +
    ' importados da auditoria comunitária · ' + counts.skipped + ' fontes não copiadas';
  $('#vendorGrid').innerHTML = state.vendors.slice(0,18).map((source) =>
    '<a class="project-card" href="' + escapeAttribute(source.url) + '" target="_blank" rel="noreferrer">' +
    '<span class="source">' + escapeHtml(source.license_spdx || 'licença preservada') + '</span>' +
    '<h3>' + escapeHtml(source.repository) + '</h3><p>Snapshot local ↗</p></a>'
  ).join('');
}

function updateProgress() {
  const done = state.curriculum.filter((module) => localStorage.getItem('cc0:' + module.id) === 'done').length;
  const percent = state.curriculum.length ? Math.round((done / state.curriculum.length) * 100) : 0;
  $('#progressText').textContent = percent + '% concluído · ' + done + '/' + state.curriculum.length + ' módulos';
  $('#progressBar').style.width = percent + '%';
}
function escapeHtml(value){return String(value).replace(/[&<>"']/g,(c)=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#039;'}[c]));}
function escapeAttribute(value){return escapeHtml(value);}

$('#levelFilters').addEventListener('click',(event)=>{
  const button=event.target.closest('[data-level]'); if(!button)return;
  state.level=button.dataset.level;
  $$('#levelFilters .filter').forEach((item)=>item.classList.toggle('active',item===button));
  renderCurriculum();
});
$('#lessonSearch').addEventListener('input',(event)=>{
  state.lessonQuery=event.target.value; state.visibleLessons=24; renderLessons();
});
$('#lessonModuleFilter').addEventListener('change',(event)=>{
  state.lessonModule=event.target.value; state.visibleLessons=24; renderLessons();
});
$('#lessonLoadMore').addEventListener('click',()=>{
  state.visibleLessons+=24; renderLessons();
});
$('#projectSearch').addEventListener('input',(event)=>{state.projectQuery=event.target.value;state.visibleProjects=36;renderProjects();});
$('#sourceFilter').addEventListener('change',(event)=>{state.source=event.target.value;state.visibleProjects=36;renderProjects();});
$('#categoryFilter').addEventListener('change',(event)=>{state.category=event.target.value;state.visibleProjects=36;renderProjects();});
$('#loadMore').addEventListener('click',()=>{state.visibleProjects+=36;renderProjects();});
$('#globalSearch').addEventListener('input',(event)=>{
  const q=event.target.value.trim(); if(!q)return;
  state.projectQuery=q; $('#projectSearch').value=q; location.hash='#projetos'; renderProjects();
});
loadData().catch((error)=>{$('#projectMeta').textContent=error.message;console.error(error);});
