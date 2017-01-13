/**
 * @description model 文件，用来放置所有与数据相关的操作。包括 数据本身、reducers、effects 等。
 * @author amas
 */

 import {getRPC} from '../../../common/util'

export default {
  namespace: 'amas',
  state: {
    status: 'ALL',
    data : {},
  },
  reducers: {
    initListDataSucess(state, {playload}) {
      return {...state, data: playload};
    },
    tabChanged(state, {playload}) {
      return {...state, status: playload};
    },
  },
  effects: {
    * initListData(action, { call, put }) {
       var data = yield call(fetch, 'alipay.pcredit.huabei.prod.list', {}, true);
       yield put({ type: 'initListDataSucess', playload : data});
    },
  },
  subscriptions: {},
};


// ---------
// Helpers


function fetch(url, param, isShowLoading) {
  return new Promise((resolve, reject) => {
    getRPC(url, param, function(data){
      resolve(data);
    }, (err)=>{
      data.url = url;
     reject(data);
     }, isShowLoading);
  });
}

function delay(timeout) {
  return new Promise(resolve => {
    setTimeout(resolve, timeout);
  });
}
