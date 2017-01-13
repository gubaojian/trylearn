/**
 * @file 棱镜配置文件
 */

var pkg = require('./package.json');

module.exports = {
  appid: pkg.appid,
  name: pkg.name,
  desc: pkg.description,
  host: `https://${pkg.appid}.h5app.alipay.com/www/`,
  pages: [
    {
      entry: 'index.html',
      src: './src/pages/index/index.js',
      title: '首页',
      desc: '首页 - 描述'
     },
     {
       entry: 'list.html',
        src : './src/pages/list/list.js',
        title: 'list列表',
        desc: 'list列表 - list列表页'
     }
  ]
};
