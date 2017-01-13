/**
 * @description model 文件，用来放置所有与数据相关的操作。包括 数据本身、reducers、effects 等。
 * @author amas
 */

export default {
  namespace: 'userinfo',
  state: {
    data: null,
  },
  reducers: {
    ['userinfo/loader'](state) {
      return {
        ...state,
      };
    },
  },
};
