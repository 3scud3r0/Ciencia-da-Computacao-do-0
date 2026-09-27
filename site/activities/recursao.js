'use strict';

const demoForm = document.querySelector('#recursionForm');
const demoInput = document.querySelector('#recursionInput');
const result = document.querySelector('#recursionResult');
const quiz = document.querySelector('#recursionQuiz');
const answer = document.querySelector('#quizAnswer');
const feedback = document.querySelector('#quizFeedback');
const exerciseKey = `cc0:exercise:${document.body.dataset.lessonPath}`;

function traceSum(n, events) {
  events.push(`entra: soma(${n})`);
  if (n === 0) {
    events.push('retorna: soma(0) = 0');
    return 0;
  }
  const sum = n + traceSum(n - 1, events);
  events.push(`retorna: soma(${n}) = ${sum}`);
  return sum;
}

demoForm.addEventListener('submit', (event) => {
  event.preventDefault();
  const n = Number(demoInput.value);
  if (!Number.isInteger(n) || n < 0 || n > 12) {
    result.textContent = 'Use um inteiro entre 0 e 12.';
    return;
  }
  const events = [];
  traceSum(n, events);
  result.replaceChildren();
  const heading = document.createElement('strong');
  heading.textContent = `Resultado: ${n * (n + 1) / 2}`;
  const steps = document.createElement('ol');
  for (const entry of events) {
    const item = document.createElement('li');
    item.textContent = entry;
    steps.append(item);
  }
  result.append(heading, steps);
});

quiz.addEventListener('submit', (event) => {
  event.preventDefault();
  if (Number(answer.value) === 15 && answer.value.trim() !== '') {
    localStorage.setItem(exerciseKey, 'done');
    feedback.textContent = 'Correto: 5 + 4 + 3 + 2 + 1 + 0 = 15. Você pode concluir a aula.';
    window.dispatchEvent(new Event('lesson:exercise-complete'));
  } else {
    feedback.textContent = 'Ainda não. Comece pelo caso base (zero) e some os retornos até chegar a cinco.';
  }
});

if (localStorage.getItem(exerciseKey) === 'done') {
  feedback.textContent = 'Exercício concluído. Você pode marcar a aula como concluída.';
}
