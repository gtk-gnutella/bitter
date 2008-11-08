#! /bin/sh

fail() {
  echo "CHECK $1 FAILED"
  exit 1
}

check() {
  expect_check=`echo | awk "{ print ${last_check} + 1 }"`
  if [ 0$1 -ne 0${expect_check} ]; then
    echo "WARNING: wrong numeration of test case: $1 after ${last_check}"
  fi
  last_check=$1

  if [ "$2" != "$3" ]; then
    echo "expected: \"$3\""
    echo "got:      \"$2\""
    fail "$1"
  fi
}

last_check=0
executable="src/bitter"
bitprint="${executable}"
sha1="${executable} -S"
tth="${executable} -T"

if [ ! -x "${executable}" ]; then
  echo "${executable}: not found or not executable"
  exit 1
fi

# Check empty file
res=$($bitprint /dev/null)
right='/dev/null: urn:bitprint:3I42H3S6NNFQ2MSVX7XZKYAYSCX5QBYJ.LWPNACQDBZRYXW3VHJVCJ64QBZNGHOHHHZWCLNQ'
check 1 "$res" "$right"

res=$($sha1 /dev/null)
right='/dev/null: urn:sha1:3I42H3S6NNFQ2MSVX7XZKYAYSCX5QBYJ'
check 2 "$res" "$right"

res=$($tth /dev/null)
right='/dev/null: urn:tree:tiger:LWPNACQDBZRYXW3VHJVCJ64QBZNGHOHHHZWCLNQ'
check 3 "$res" "$right"


# Check empty file over pipe
res=$(cat /dev/null | $bitprint)
right='urn:bitprint:3I42H3S6NNFQ2MSVX7XZKYAYSCX5QBYJ.LWPNACQDBZRYXW3VHJVCJ64QBZNGHOHHHZWCLNQ'
check 4 "$res" "$right"

# Check some text
text='0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.-_^!$%&|'
right='urn:bitprint:4XEIEYAUWOP3F4ZJL3LGKG3LAL77SYO6.ZFCZ4QDNMWCAXVUHG57UOAYCLBBO4SV5QYJIGXY'
res=$(echo "${text}" | $bitprint)
check 5 "$res" "$right"

# A single zero byte
right='urn:bitprint:LOUTZHNQZ74T6UVVEHLUEDSD63W2E6CP.VK54ZIEEVTWNAUI5D5RDFIL37LX2IQNSTAXFKSA'
res=$(printf "\0" | $bitprint)
check 6 "$res" "$right"

# 1024 x 'A'
text='AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA' # 64
text="${text}${text}${text}${text}${text}${text}${text}${text}" # 64 x 8
text="${text}${text}" # 512 x 2

right='urn:bitprint:ORWD6TJINRJR4BS6RL3W4CWAQ2EDDRVU.L66Q4YVNAFWVS23X2HJIRA5ZJ7WXR3F26RSASFA'
res=$(printf "${text}" | $bitprint)
check 7 "$res" "$right"

# 1025 x 'A'
text="${text}A"
right='urn:bitprint:UUHHSQPHQXN5X6EMYK6CD7IJ7BHZTE77.PZMRYHGY6LTBEH63ZWAHDORHSYTLO4LEFUIKHWY'
res=$(printf "${text}" | $bitprint)
check 8 "$res" "$right"

right='LICENSE: urn:bitprint:UZHW2ANBQXREWV7GQSX6PSFOQBGM46U2.Z6C7VFL73OZ7QDETRKP743EIL34MOCMJV3ZXPCA'
res=$($bitprint LICENSE)
check 9 "$res" "$right"

echo "ALL ${last_check} CHECKS PASSED"
exit

# vi: set ai et sts=2 sw=2:
