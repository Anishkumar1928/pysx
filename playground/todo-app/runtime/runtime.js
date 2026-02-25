// runtime/runtime.js

// -----------------------------
// Hook wrappers (simple bridge)
// -----------------------------
function useState(initial) {
  return React.useState(initial);
}

function useEffect(effect, deps) {
  return React.useEffect(effect, deps);
}

// -----------------------------
// App runner
// -----------------------------
function run(App) {

  const rootElement =
    document.getElementById("root") ||
    document.body;

  const root = ReactDOM.createRoot(rootElement);

  root.render(
    React.createElement(App)
  );
}

// -----------------------------
// Global API
// -----------------------------
window.ReactXPy = {
  run,
  useState,
  useEffect
};
