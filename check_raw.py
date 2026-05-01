# 我们需要看原始 buffer 在 POST_PUSH 写入 "" 时的内容
# 但输出文件是 phase6 处理后的，所以需要重新预处理并捕获 raw buffer
# 我们可以用 -P 参数抑制 linemarker，看看 "" 是否保留
