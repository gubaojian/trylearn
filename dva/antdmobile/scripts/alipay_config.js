const webpack = require('atool-build/lib/webpack');
const path = require('path');
const pkg = require('../package.json');

const lessAlias = require('./less_alias');
const jsAlias = require('./js_alias');

const alipayConfig = function(webpackConfig) {

  const isDev = process.env.NODE_ENV === 'development';
  const isMock = process.env.NODE_ENV === 'mock';

  /**
   * Less Alias
   */
  lessAlias(webpackConfig, {
    'THEME': path.join(__dirname, '../src/config/alipay/theme'),
  });

  /**
   * JS Alias
   */
  jsAlias(webpackConfig, [
    {
      src: isMock ? './mock/mockGetData' : './src/common/util/getData',
      expose: 'GETDATA_ADAPTER',
    }, {
      src: './src/config/alipay/bridge',
      expose: 'BRIDGE',
    },
  ]);

  /**
   * 条件变量声明
   */
  webpackConfig.plugins.push(
    new webpack.DefinePlugin({
      // 在rpc里面统一注入appid
      APP_ID: JSON.stringify(pkg.appid),
      // rpc里面统一注入appVersion
      APP_VERSION: JSON.stringify(pkg.version),
      // 方便控制台里面调试rpc返回的数据
      DEVLOG: `${!isDev} || console.log(result)`,
    })
  );

  /**
   * UI库配置
   */
  webpackConfig.babel.plugins.push([
    'antd',
    {
      style: true,
      libraryName: 'antd-mobile',
    },
  ]);
};

module.exports = alipayConfig;
