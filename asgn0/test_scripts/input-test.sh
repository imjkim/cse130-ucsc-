# Tests for split on incorrect usage (wonderful.txt)
# Test should output the same as ./rsplit binary in order to pass
text="test_files/wonderful.txt"
diff <(./split $text) <(./rsplit $text) > /dev/null
