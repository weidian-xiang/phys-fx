# 解析器模糊测试语料

`edit-script`、`.physfxproj` 和 `.pfxtpl` 的入口都先经过有资源上限的 JSON 解析器。
此目录保留最小语料，nightly 会运行 `tools/fuzz_json.py`，对语料做确定性变异并确认
解析器只返回错误或成功，不崩溃、不无限耗时。独立 fuzz runner 可将同一输入接到
libFuzzer/AFL++；本仓库不要求 PR 机器安装这些工具。
