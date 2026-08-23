# Demo Applications

Demo applications showcase complete production-oriented projects built with Drogular.

Current demos:

- Portal Demo
- Store Demo (planned)
- Chat Demo (planned)

## Portal Demo template DSL

Portal Demo is also the reference application for Drogular's template expression
features. Its templates use application expression functions for localization
(`t()`), lexical `@let`/`@const` bindings, collection helpers, list membership,
and `@switch` for presentation-only state formatting. Page code keeps
request/domain preparation in C++ while translation keys and presentation
choices stay in templates.
