# htmpl.h — Lightweight C Header-Only HTML Templating

`htmpl` is a lightweight, header-only C library for generating HTML in C programs using macros and string builders. It also includes a template compiler that compacts and transforms `.html`-like templates into C macros for fast and efficient embedding.

Template:
```
<!DOCTYPE html>
<head>
    <title>Website</title>
</head>
<body>
    <div style="border: 1px solid #000">
        <h1>Demo</h1>
        $ {
            HTML("<h2>Hi, %s</h2>", name);
        }
    </div>
        
    $ {
        HTML("<h3>Number of items: %i</h3>", cnt);
        for (int i = 0; i < cnt; i++) {
            HTML(`
                <p style="color: red;">Item № \`%i\`</p>
            `, i + 1);
        }
    }
</body>
</html>
```

Rendered page:
```
<!DOCTYPE html>
<head>
  <title>Website</title>
</head>
<body>
  <div style="border: 1px solid #000">
    <h1>Demo</h1>
    <h2>Hi, Michael</h2>
  </div>
  <h3>Number of items: 8</h3>
      <p>Item № 1</p>
      <p>Item № 2</p>
      <p>Item № 3</p>
      <p>Item № 4</p>
      <p>Item № 5</p>
      <p>Item № 6</p>
      <p>Item № 7</p>
      <p>Item № 8</p>
</body>
</html>
```

## Features

- **Header-only:** Just include the header in your project.
- **Simple template macro:** Write `HTML("...")` or ```HTML(`...`)``` blocks inline in your C code.
- **Template compiler:** Convert HTML templates into C macros at compile time.
- **Minimal dependencies:** Only relies on the standard C library.
- **StringBuilder utilities:** Efficient string building with dynamic resizing.

## Files

- `htmpl.h` – Main header-only library (you include this).
- `demo/templates.c` – Example of template compilation.
- `demo/main.c` – Example of generated template usage.

## Try the demo

You can build and try the example using the following commands:
```
# build and run the demo
chmod +x ./run.sh
./run.sh

# open rendered page
firefox ./index.html
```
