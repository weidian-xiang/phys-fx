<!--
 PhysFX Engine —— 视频世界编辑器（Video World Editor）
 Copyright (c) 2026 向伟典
 SPDX-License-Identifier: Apache-2.0
-->

# 编辑脚本格式

编辑脚本是开发者入口，模板命令会在运行时生成等价脚本。顶层必须包含 `version` 和
`commands`；当前版本为 `1`。每个命令必须包含 `type`，实体命令还必须包含 `entity_id`。

```json
{
  "version": 1,
  "commands": [
    {"type": "select_entity", "entity_id": 1},
    {"type": "move_entity", "entity_id": 1, "x": 520, "y": 300},
    {"type": "copy_entity", "entity_id": 1}
  ]
}
```

支持的命令：`empty`、`select_entity`、`delete_entity`、`move_entity`、`copy_entity`、
`change_material`（当前支持 `material` 名称 `red`/`blue`/`green`）。未知命令、缺字段或
不支持的版本会返回中文错误并停止管线，不会静默忽略。
