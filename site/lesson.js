'use strict';

const lessonPath = document.body.dataset.lessonPath;
const completeButton = document.querySelector('#lessonComplete');
const progressKey = `cc0:lesson:${lessonPath}`;

function refreshLessonProgress() {
  const done = localStorage.getItem(progressKey) === 'done';
  completeButton.setAttribute('aria-pressed', String(done));
  completeButton.textContent = done ? 'Concluída ✓ · desfazer' : 'Marcar como concluída';
}

completeButton.addEventListener('click', () => {
  if (localStorage.getItem(progressKey) === 'done') {
    localStorage.removeItem(progressKey);
  } else {
    localStorage.setItem(progressKey, 'done');
  }
  refreshLessonProgress();
});

refreshLessonProgress();
