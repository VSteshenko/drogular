# `PwaScripts`

**Namespace:** `drogular`  
**Header:** `<drogular/pwa_scripts.hpp>`  
**Kind:** Utility class

## Purpose

`PwaScripts` generates small browser-side JavaScript snippets for service-worker registration, local input persistence, and online/offline UI state.

---

## Public API

### `serviceWorkerRegistration()`

```cpp
static std::string serviceWorkerRegistration(
    const std::string& path =
        "/service-worker.js"
);
```

Generates a `<script>` that:

1. checks whether `navigator.serviceWorker` is available;
2. waits for the window `load` event;
3. calls `navigator.serviceWorker.register(path)`.

The default path matches the fixed route exposed by `App::serviceWorker()`.

A custom path changes only the JavaScript registration URL. It does not register a matching server route.

### `inputPersistence()`

```cpp
static std::string inputPersistence(
    const std::string& inputId,
    const std::string& storageKey
);
```

Generates a script that:

- looks up the element with `document.getElementById(inputId)`;
- restores an existing `localStorage` value into `input.value`;
- stores the current value on every `input` event;
- removes the stored value when the input's form is submitted.

If the element is not present when the script executes, the script returns immediately and installs no listeners.

### `offlineStatus()`

```cpp
static std::string offlineStatus(
    const std::string& elementId
);
```

Generates a script that:

- looks up the target element;
- hides it when `navigator.onLine` is `true`;
- shows it when offline;
- updates the state in response to browser `online` and `offline` events.

If the target element is not present when the script executes, no listeners are installed.

---

## Behavior

All arguments are inserted directly into JavaScript string literals without JavaScript escaping.

Use trusted application-controlled identifiers and paths. Do not pass untrusted request values directly to these helpers.

`inputPersistence()` and `offlineStatus()` execute their element lookup immediately. Place the generated scripts after the corresponding DOM elements, or otherwise ensure those elements already exist at execution time.

`inputPersistence()` assumes the target element exposes a writable `value` property. It does not catch `localStorage` access errors.

`offlineStatus()` reflects the browser's `navigator.onLine` state; it does not perform an application-level connectivity check.

---

## Example

TodoPWA adds both helpers to its page context:

```cpp
context.set(
    "todoDraftPersistence",
    drogular::PwaScripts::inputPersistence(
        "todo-title",
        "todo-draft"
    )
);

context.set(
    "offlineStatusScript",
    drogular::PwaScripts::offlineStatus(
        "offline-status"
    )
);
```

---

## Related Types

- [`PwaOptions`](pwa-options.md)
- [`PwaHtml`](pwa-html.md)
- [`PwaPageSupport`](../pages/pwa-page-support.md)
- [`App`](../application/app.md)
