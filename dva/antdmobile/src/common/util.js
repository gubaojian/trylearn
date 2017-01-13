/**
 * @file util
 * @author anima
 */
import getJSON from '@ali/anima-getjson';
import mockData from 'GETDATA_ADAPTER'; // GETDATA_ADAPTER是在webpack里配置的alias,切换rpc的实现

// 全局配置, 方便换线上rpc, 以及mock;
getJSON.config({
  // getData方法只处理获取数据逻辑, 不处理任何其他逻辑;
  getData: mockData,
});

const getRPC = function(url, param, fetchSuccess, fetchError, isShowLoading = true) {
  getJSON.factory({
    interface: url,
    param: param || {},
    success: fetchSuccess,
    error: fetchError,
    beforeSend: () => {
      if (isShowLoading) {
        Ali.showLoading();
      }
    },
    beforeComplete: () => {
      if (isShowLoading) {
        Ali.hideLoading();
      }
    },
  });
};

export default {
  getRPC,
};
