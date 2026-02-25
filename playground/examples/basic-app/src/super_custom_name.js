import Header from "./Header.js";

function App() {
  return React.createElement("div", { className: "box" },
    React.createElement(Header, { className: "box" }),
    "Hello  ",
    user,
    React.createElement(Header, null));
}

export default App;
