// Front end for the WebAssembly build. All search logic lives in C++ (core/).
import createAlgoViz from './algoviz.js';

const COLS = 40;
const ROWS = 30;
const CELL = 20;
const SPEEDS = [1, 2, 4, 8, 16, 64, -1]; // steps per frame; -1 = instant
const SPEED_LABELS = ['1/frame', '2/frame', '4/frame', '8/frame', '16/frame', '64/frame', 'instant'];
const ALGO_NAMES = ['BFS', 'DFS', 'Dijkstra', 'A*', 'Greedy Best-First'];

// Codes from web/bindings.cpp
const COLORS = [
  '#14143F', // empty
  '#D9D9F0', // wall
  '#5B16C9', // weight
  '#FF4FD8', // frontier
  '#1652B8', // visited
  '#F7A21B', // path
  '#3EE07A', // start
  '#FF7A45', // goal
  '#3A30C0', // visited weight
];

const $ = (id) => document.getElementById(id);
const canvas = $('grid');
const ctx = canvas.getContext('2d');
canvas.width = COLS * CELL;
canvas.height = ROWS * CELL;

let engine;
let running = false;
let seed = 2026;
let drag = null; // 'paint' | 'erase' | 'start' | 'goal'

const algo = () => Number($('algo').value);
const speed = () => SPEEDS[Number($('speed').value)];
const paintType = () => Number(document.querySelector('input[name=mode]:checked').value);

function setRunning(on) {
  running = on;
  $('run').textContent = on ? 'Pause' : 'Run';
  $('run').setAttribute('aria-pressed', String(on));
}

function freshSearchIfDone() {
  const st = engine.status();
  if (st === 1 || st === 2) engine.reset();
  if (!engine.active()) engine.begin(algo());
}

function edit(fn) {
  setRunning(false);
  fn();
}

function draw() {
  const view = engine.view();
  for (let i = 0; i < view.length; i++) {
    ctx.fillStyle = COLORS[view[i]];
    ctx.fillRect((i % COLS) * CELL + 1, Math.floor(i / COLS) * CELL + 1, CELL - 2, CELL - 2);
  }
  const st = engine.status();
  const status = $('sStatus');
  const text = st === -1 ? 'Idle' : running ? 'Running' : st === 1 ? 'Found' : st === 2 ? 'No path' : 'Paused';
  if (status.textContent !== text) status.textContent = text;
  status.className = st === 1 ? 'found' : st === 2 ? 'nopath' : '';
  $('sVisited').textContent = engine.visited();
  $('sFrontier').textContent = engine.frontier();
  $('sLen').textContent = engine.pathLength();
  $('sCost').textContent = engine.pathCost();
  $('sSteps').textContent = engine.steps();
  $('sOptimal').textContent = st === 1 ? (engine.optimal() ? 'Guaranteed' : 'Not guaranteed') : '–';
}

function frame() {
  if (running) {
    const st = engine.step(speed());
    if (st !== 0) setRunning(false);
  }
  draw();
  requestAnimationFrame(frame);
}

// ---------------------------------------------------------------- pointer input (mouse + touch)
function cellFromEvent(e) {
  const r = canvas.getBoundingClientRect();
  const x = Math.floor(((e.clientX - r.left) / r.width) * COLS);
  const y = Math.floor(((e.clientY - r.top) / r.height) * ROWS);
  if (x < 0 || y < 0 || x >= COLS || y >= ROWS) return -1;
  return y * COLS + x;
}

function applyDrag(i) {
  if (i < 0) return;
  if (drag === 'start') engine.setStart(i);
  else if (drag === 'goal') engine.setGoal(i);
  else if (drag === 'erase') engine.setCell(i, 0);
  else if (drag === 'paint') engine.setCell(i, paintType());
}

canvas.addEventListener('pointerdown', (e) => {
  const i = cellFromEvent(e);
  if (i < 0) return;
  canvas.setPointerCapture(e.pointerId);
  edit(() => {
    if (i === engine.start()) drag = 'start';
    else if (i === engine.goal()) drag = 'goal';
    else drag = e.button === 2 || paintType() === 0 ? 'erase' : 'paint';
    applyDrag(i);
  });
  e.preventDefault();
});
canvas.addEventListener('pointermove', (e) => { if (drag) applyDrag(cellFromEvent(e)); });
const endDrag = () => { drag = null; };
canvas.addEventListener('pointerup', endDrag);
canvas.addEventListener('pointercancel', endDrag);
canvas.addEventListener('contextmenu', (e) => e.preventDefault());

// ---------------------------------------------------------------- controls
function toggleRun() {
  freshSearchIfDone();
  setRunning(!running);
}
function stepOnce() {
  freshSearchIfDone();
  setRunning(false);
  engine.step(1);
}
const actions = {
  maze: () => edit(() => engine.maze(seed++)),
  walls: () => edit(() => engine.scatterWalls(0.3, seed++)),
  weights: () => edit(() => engine.scatterWeights(0.15, seed++)),
  clearSearch: () => edit(() => engine.reset()),
  clearAll: () => edit(() => engine.clearAll()),
};

function updateSpeedLabel() { $('speedOut').textContent = SPEED_LABELS[Number($('speed').value)]; }

function wire() {
  $('run').addEventListener('click', toggleRun);
  $('step').addEventListener('click', stepOnce);
  for (const [id, fn] of Object.entries(actions)) $(id).addEventListener('click', fn);
  $('algo').addEventListener('change', () => edit(() => engine.reset()));
  $('speed').addEventListener('input', updateSpeedLabel);
  updateSpeedLabel();

  document.addEventListener('keydown', (e) => {
    const tag = e.target.tagName;
    if (tag === 'SELECT' || tag === 'INPUT' || tag === 'BUTTON' || e.ctrlKey || e.metaKey || e.altKey) return;
    const k = e.key.toLowerCase();
    if (k >= '1' && k <= '5') { $('algo').value = String(Number(k) - 1); edit(() => engine.reset()); }
    else if (k === ' ') toggleRun();
    else if (k === 's') stepOnce();
    else if (k === 'm') actions.maze();
    else if (k === 'r') actions.walls();
    else if (k === 't') actions.weights();
    else if (k === 'c') (e.shiftKey ? actions.clearAll : actions.clearSearch)();
    else if (k === 'arrowup') { $('speed').stepUp(); updateSpeedLabel(); }
    else if (k === 'arrowdown') { $('speed').stepDown(); updateSpeedLabel(); }
    else return;
    e.preventDefault();
  });
}

createAlgoViz()
  .then((Module) => {
    engine = new Module.Engine(COLS, ROWS);
    engine.maze(seed++);
    wire();
    window.algoviz = { engine, names: ALGO_NAMES }; // handy for automated checks
    requestAnimationFrame(frame);
  })
  .catch((err) => {
    document.querySelector('main').insertAdjacentHTML(
      'afterbegin',
      `<p class="err">Couldn't load the WebAssembly engine: ${String(err)}</p>`,
    );
  });
