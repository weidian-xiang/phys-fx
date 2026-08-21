# Edit Script Format

An edit script is a JSON object with integer `version: 1` and a `commands` array. Every command
is an object with a string `type`; malformed JSON, duplicate keys, excessive depth and oversized
files are rejected before execution.

Keyframe animation uses `animate_parameter`, an explicit `linear` or `smooth` interpolation and
strictly increasing `{frame, value}` keyframes. GUI and CLI serialize the same command contract.
