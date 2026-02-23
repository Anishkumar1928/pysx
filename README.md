# 🚀 ReactXPy (Python Syntax for React)

Welcome to **ReactXPy**, a revolutionary framework that bridges the elegance of Python with the raw power of the React component ecosystem. ReactXPy is an ultra-fast, C++ based transpiler that allows developers to write feature-rich, interactive web applications using clean and readable Python syntax, entirely eliminating the configuration hell of modern JavaScript bundles.

If you love Python's indentation-based readability but need the dynamic, component-driven architecture of React, ReactXPy gives you both without needing Webpack, Vite, Babel, or Node.js running on your machine.

---

## ✨ Core Features (v2)

- **Pure Python Syntax for Components:** Define your UI components using standard `def ComponentName(props):` definitions, free of brackets and JavaScript boilerplate.
- **Native Inline JSX:** Seamlessly blend standard HTML/JSX tags directly inside your Python functions. The C++ parser intelligently distinguishes Python control flow from DOM declarations.
- **State & Hooks Engine (NEW):** ReactXPy v2 natively supports React Hooks! Utilize `useState` and `useEffect` with standard Python assignments and lambdas.
- **Dynamic Props & Interpolation:** Pass data across components naturally using `{props.value}` syntax. Evaluate complex math or logic inside interpolations instantly.
- **Native Event Handling:** Bind Python functions directly to DOM events like `onClick={triggerFunc}` without dealing with `this` binding or context scoping.
- **Powerful Conditional Rendering:** Drive UI states with intuitive boolean logic such as `{isActive && <div class="badge">Online</div>}` right inside the tree.
- **Zero-Dependency CLI Scaffold:** Get started in zero seconds. The built-in `create-reactxpy-app` generator instantly architectures a complete project for you.
- **Cross-Platform C++ Core:** The transpiler is compiled natively onto your machine (Windows `.exe`, macOS, or Linux) enabling lightning-fast, concurrent build times.
- **Local Dev Server Hook:** ReactXPy ships with a multi-threaded, hot-reloading native Python watcher (`dev.py`), replacing the need for NPM servers.

---

## 📦 Installation

ReactXPy is distributed globally via standard Python pip packages. Before beginning, ensure you are running Python 3.7+.

To install the framework globally onto your machine:

```bash
pip install reactxpy
```

Verify your installation was successful and the native C++ binary was compiled by checking the CLI tool:

```bash
reactxpy --version
# Output: reactxpy version 0.1.0
```

---

## 🛠️ Quick Start

ReactXPy ships with a state-of-the-art interactive CLI to generate standalone applications mimicking the best practices of modern frontend development.

1. **Scaffold a Project**
   Run the project generator from anywhere in your terminal:

   ```bash
   create-reactxpy-app my-awesome-app
   ```

2. **Launch the Live Server**
   Navigate into your new application and execute the pure-Python hot reloader:
   ```bash
   cd my-awesome-app
   python3 dev.py
   ```

**That's it!** The `dev.py` script automatically watches your `src/` directory for filesystem changes. It instantly hot-recompiles your `.reactxpy` components into deeply-linked browser-safe Javascript bundles (`dist/bundle.js`) and serves it live at `http://localhost:3000`.

---

## 📖 Comprehensive Syntax Guide

### 1. Components & Standard JSX

ReactXPy treats components naturally. You declare them identically to standard Python functions. When you return HTML DOM tags, the compiler natively intercepts it and translates the structure into optimized `React.createElement` syntax trees.

```python
# Greet.reactxpy
def Greet(name):
    return <div class="greeting-card">
        <h3>Hello {name}!</h3>
        <p>Welcome to the ReactXPy ecosystem.</p>
    </div>
```

### 2. State Hooks (`useState` & `useEffect`)

ReactXPy v2 fully embraces the React runtime. You can manage encapsulated component states easily by destructuring `useState` tuples, and orchestrate browser lifecycle features like API calls or Local Storage using `useEffect`.

```python
# CounterApp.reactxpy
def CounterApp():
    # Native Python Tuple Destructuring
    count, setCount = useState(0)

    # Persist the count to the browser's storage whenever the `count` dependency updates
    useEffect(lambda: localStorage.setItem("clicks", JSON.stringify(count)), [count])

    def triggerIncrement():
        setCount(count + 1)

    return <div class="counter-panel">
        <p>Total Clicks: {count}</p>
        <button onClick={triggerIncrement}>Increment Value</button>
    </div>
```

### 3. Conditional UI & Component Composition

ReactXPy supports infinitely nested component architectures. You can import modules seamlessly without Node syntax. It also securely translates HTML attributes (like `class`) into DOM-safe syntax (`className`) automatically.

```python
# App.reactxpy
import CounterApp

def App():
    isAdmin = True

    return <main class="dashboard-wrapper">
        <h1>Admin Control Panel</h1>

        {isAdmin && <div class="secure-badge">Verified Session</div>}

        <div class="widgets">
            <CounterApp />
        </div>
    </main>
```

---

## ⚙️ How the Architecture Works

1. **Native OS Compilation:** When you `pip install reactxpy`, your computer leverages its local `g++` compiler to build a hyper-optimized `.exe` (Windows) or binary (Unix) of the ReactXPy transpiler directly within your Python packages folder.
2. **Lexical Analysis Engine:** The C++ compiler scans your `.reactxpy` files. It fundamentally understands the boundary between Python variable assignments, indentations, scopes, and HTML tag delimiters.
3. **AST Transpilation:** The framework safely merges Javascript operators (like `||` and `&&`), mathematical calculations, and JSX scopes into raw, standalone Javascript mappings.
4. **Dependency-Free Bundle:** The compiler bridges your states via a lightweight `runtime.js` hook memory wrapper. It drops ES module syntax entirely—so you don't even need a server! You can double-click `index.html` directly from a USB stick and the app will flawlessly function offline in standard browsers.

---

## 🔮 Development Roadmap

ReactXPy is an actively maintained open-source framework. The current horizon features include:

- [x] Transpilation of Standard JSX directly to `React` API calls.
- [x] Interactive State Management (`useState` Hooks).
- [x] Application Lifecycle Memory (`useEffect` Hooks).
- [x] Zero-Dependency Build Pipelines (`build.py` & `dev.py`).
- [x] Native Microsoft Windows (`.exe`) Subprocess Compilation.
- [ ] List Comprehension Syntax (e.g., `<item for item in array>`).
- [ ] Asynchronous API Fetching (`async/await` components).
- [ ] Deep Integrated Server-Side Python Routing.

---


### License

MIT License - Have fun building!
