import hashlib

target = "45dfe2341cc204b69d0e0af63b8d8ae3"
original = bytes(range(256))

def check(name, data):
    m = hashlib.md5(data).hexdigest()
    if m == target:
        print(f"MATCH: {name}")
        return True
    return False

# 1. Identity
check("Identity", original)

# 2. \n -> \r\n
data = original.replace(b"\n", b"\r\n")
check("LF -> CRLF", data)

# 3. \r\n -> \n
data = original.replace(b"\r\n", b"\n")
check("CRLF -> LF", data)

# 4. \r -> \n
data = original.replace(b"\r", b"\n")
check("CR -> LF", data)

# 5. \n -> \r
data = original.replace(b"\n", b"\r")
check("LF -> CR", data)

# 6. Strip \r
data = original.replace(b"\r", b"")
check("Strip CR", data)

# 7. Strip \n
data = original.replace(b"\n", b"")
check("Strip LF", data)

# 8. Missing last byte (0xFF)
data = original[:-1]
check("Missing last byte", data)

# 10. Last byte modified
for i in range(256):
    data = list(original)
    data[-1] = i
    check(f"Last byte -> {i:02x}", bytes(data))

# 11. Any single byte modified
for pos in range(256):
    for val in range(256):
        data = list(original)
        data[pos] = val
        if check(f"Byte {pos:02x} -> {val:02x}", bytes(data)):
            exit()

# 12. Telnet IAC (0xFF) doubled?
data = original.replace(b"\xff", b"\xff\xff")
check("Double IAC", data)

# 13. Telnet IAC (0xFF) stripped?
data = original.replace(b"\xff", b"")
check("Strip IAC", data)

# 15. Prepend \n (0x0A) and drop last
data = b"\n" + original[:-1]
check("Prepend \\n, drop last", data)

# 16. Prepend \r\n (0x0D 0x0A) and drop last 2
data = b"\r\n" + original[:-2]
check("Prepend \\r\\n, drop last 2", data)

# 17. Prepend \r (0x0D) and drop last
data = b"\r" + original[:-1]
check("Prepend \\r, drop last", data)

# 18. Shift right 1 (prepend 0x00, drop last)
data = b"\x00" + original[:-1]
check("Prepend 0x00, drop last", data)

# 19. Replace \xff with nothing (size 255) - wait, size is 256. 
# Maybe \xff became \r\n? (Size 257)
# Maybe \xff became \n?
data = original.replace(b"\xff", b"\n")
check("Replace \\xff with \\n", data)

# 20. Replace \xff with \r
data = original.replace(b"\xff", b"\r")
check("Replace \\xff with \\r", data)

# 21. Check for double newline at start
# The log showed \r\n\n\x00. 
# If storage read output format is Size: 256\r\n<DATA>, then data starts with \n\x00.
# Original starts with \x00. So \n was inserted.
# If \n inserted, and size is 256, last byte must be lost.
# This matches test #15.

print("Done.")
