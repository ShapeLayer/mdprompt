const assert = require('node:assert/strict');
const { plainCodeBlocks } = require('../../editors/vscode/plain-code');
const ranges = text => plainCodeBlocks(text.split('\n'));
assert.deepEqual(ranges('```mdprompt\n가나다^%코드 안에서는 지시가 아니다.%^ 줄글텍스트\n일반 텍스트\n```\n^%실제 지시%^'), [{start:1,end:3}]);
assert.deepEqual(ranges('~~~~mdprompt\nraw\n~~~\nstill raw\n~~~~~'), [{start:1,end:4}]);
assert.deepEqual(ranges('```mdprompt\nraw\n~~~\nstill raw\n```'), [{start:1,end:4}]);
assert.deepEqual(ranges('```\nunclosed'), [{start:1,end:2}]);
assert.deepEqual(ranges('```\n```'), [{start:1,end:1}]);
assert.deepEqual(ranges('    ```\nnormal\n    ```'), []);
assert.deepEqual(ranges('^%%\n```\nrequest\n```\n%%^\n$$\n```\nmath\n```\n$$'), []);
assert.deepEqual(ranges('   ```mdprompt\nraw\n   ```\n~~~\nsecond\n~~~'), [{start:1,end:2},{start:4,end:5}]);
// Editing away a closing fence must immediately extend the uniform range.
assert.deepEqual(ranges('```\nraw\nclosing removed\n^%now also code%^'), [{start:1,end:4}]);
console.log('Plain code color ranges passed.');
