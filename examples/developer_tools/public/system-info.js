export function render(container, context) {
    const data = context.data ?? {};
    const list = document.createElement("dl");

    for (const [key, value] of Object.entries(data)) {
        const term = document.createElement("dt");
        term.textContent = key;
        const description = document.createElement("dd");
        description.textContent = typeof value === "string"
            ? value
            : JSON.stringify(value);
        list.append(term, description);
    }

    container.replaceChildren(list);
}
