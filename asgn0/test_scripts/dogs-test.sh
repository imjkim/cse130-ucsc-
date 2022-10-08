# Tests for split on correct behavior for ./split -
# Test should output the same as ./rsplit binary in order to pass
# There seemed to be problems with dogs.jpg file, so I made a test to see if my split works correctly with dogs.jpg.
text="test_files/dogs.jpg e"
diff <(./split $text ) <(./rsplit $text ) > /dev/null
