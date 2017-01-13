/**
 * @description model 文件，用来放置所有与数据相关的操作。包括 数据本身、reducers、effects 等。
 * @author amas
 */

export default {
  namespace: 'amas',
  state: {
    status: 'initial',
  },
  reducers: {
    changing(state) {
      return {...state, status: 'changing'};
    },
    changed(state) {
      return {...state, status: 'changed'};
    },
    reset(state) {
      return {...state, status: 'initial'};
    },
  },
  effects: {
    * change(action, { call, put }) {
      yield put({ type: 'changing' });

      yield call(delay, 1000);

      yield put({ type: 'changed' });
    },
  },
  subscriptions: {},
};


// ---------
// Helpers

function delay(timeout) {
  return new Promise(resolve => {
    setTimeout(resolve, timeout);
  });
}
