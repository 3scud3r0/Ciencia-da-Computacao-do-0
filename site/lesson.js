'use strict';

const lessonPath = document.body.dataset.lessonPath;
const completeButton = document.querySelector('#lessonComplete');
const progressKey = `cc0:lesson:${lessonPath}`;

function refreshLessonProgress() {
  const needsExercise = document.body.dataset.hasExercise === 'true';
  const solved = localStorage.getItem(`cc0:exercise:${lessonPath}`) === 'done';
  const done = localStorage.getItem(progressKey) === 'done';
  completeButton.disabled = needsExercise && !solved;
  completeButton.setAttribute('aria-pressed', String(done));
  completeButton.textContent = needsExercise && !solved ? 'Resolva o exercício para concluir' :
    done ? 'Concluída ✓ · desfazer' : 'Marcar como concluída';
}

completeButton.addEventListener('click', () => {
  if (localStorage.getItem(progressKey) === 'done') {
    localStorage.removeItem(progressKey);
  } else {
    localStorage.setItem(progressKey, 'done');
  }
  refreshLessonProgress();
});

window.addEventListener('lesson:exercise-complete', refreshLessonProgress);
refreshLessonProgress();
