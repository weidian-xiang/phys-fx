# Plugin SDK

Implement `IEffectPlugin` or `IEditTemplatePlugin`, keep metadata and command descriptors stable,
and package a manifest with `packageId`, `version` and `publisher`. `tools/pack_template.py`
creates deterministic `.pfxtpl` packages and can sign them with an offline Ed25519 key.

Unsigned packages remain usable by the open-source core. Never ship private keys, execute manifest
commands, upload user media, or advertise a documented fallback as neural inference.
