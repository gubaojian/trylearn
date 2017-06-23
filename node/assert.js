const assert = require('assert');
const a = {
   a: {
     b : 1
   }
};

const b = {
   a: {
     b : 1
   }
};


assert(a != null, "你好");

console.log(a == b)
assert.deepEqual(a, b);
console.log(assert.deepStrictEqual(a, b, "deepEqual"));


console.log(SyntaxError);

console.log(TypeError);

assert.equal(1, '1');

//assert.fail(1, 1, 'failed assert me');

//assert.ifError('error')
assert.throws(
  () => {
    throw new Error('错误信息');
  },
  function(err) {
    if ((err instanceof Error) && /错误/.test(err)) {
      return true;
    }
  },
  '不是期望的错误'
);

assert.doesNotThrow(
  () => {
    throw new TypeError('错误');
  },
  TypeError, '你好，我再测试呢？');

  console.log('go on');
