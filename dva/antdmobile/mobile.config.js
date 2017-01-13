var pkg = require('./package.json');
var hybridConfig = require('@alipay/hybrid-configuration');
var qrcode = '';

// 在这里控制ios模拟器的相关信息,如支付宝版本,模拟器版本等
var alipaySim = hybridConfig.getPortal({
  appid: pkg.appid,
  url: 'http://localhost:8001/index.html',
  device: 'iPhone-6s',
  os: '', // 不设定默认最新系统
  version: '9.6'
});
module.exports.simOpts = Object.assign({qrcode: qrcode}, alipaySim);

// open in safari
// var safariSim = {scheme: "http://127.0.0.1:8001"};
// module.exports.simOpts = Object.assign({ qrcode: qrcode}, safariSim);

// open in afwealth.app
var afwealthSim = hybridConfig.getAfwealth('http://m.baidu.com');
// module.exports.simOpts = Object.assign({qrcode: qrcode}, afwealthSim);


// open in taobao.app
var taobaoSim = hybridConfig.getTaobao('http://m.taobao.com');
// module.exports.simOpts = Object.assign({qrcode: qrcode}, taobaoSim);

module.exports.packages = {

  'portal': {
    'debug': alipaySim,
    'deploy': 'nebula',
    'container': 'AP',
    'appid': pkg.appid,
    'name': pkg.name,
    'version': '1.0.0.0',
    'description': 'socialcloud_for_portal',
    'launchParams': {
      'url': '/www/index.html',
      'showTitleBar': 'YES'
    },
    'host': {
      'enable': true,
      'dev': `http://${pkg.appid}.h5app.alipay.net`,
      'test': `https://${pkg.appid}.h5app.test.alipay.net`,
      'online': `https://${pkg.appid}.h5app.alipay.com`
    },
    'mapLocal': {
      'static.alipayobjects.com/publichome-static/antBridge': '/sdk'
    },
    'build': {
      'include': [
        './www/index.*',
        './www/detail.*',
        './www/common.*',
        './www/flex.*',
        './www/vw.*'
      ],
      'ignore': [
        './**/*.md'
      ]
    },
    'support': {
      'ios': {
        'minSDK': '0',
        'maxSDK': '100',
        'minOS': '9.6.10',
        'maxOS': '100'
      },
      'android': {
        'minSDK': '0',
        'maxSDK': '100',
        'minOS': '9.6.10',
        'maxOS': '100'
      }
    }
  },
  'afwealth': {
    'debug': afwealthSim,
    'deploy': 'nebula',
    'container': 'AF',
    'appid': pkg.appid,
    'name': pkg.name,
    'version': '1.0.0.0',
    'description': 'socialcloud_for_afwealth',
    'launchParams': {
      'url': '/www/index.html'
    },
    'host': {
      'enable': true,
      'dev': `http://${pkg.appid}.h5app.alipay.net`,
      'test': `https://${pkg.appid}.test.h5app.alipay.net`,
      'online': `https://${pkg.appid}.h5app.alipay.com`
    },
    'mapLocal': {
      'static.alipayobjects.com/publichome-static/antBridge': '/sdk'
    },
    'build': {
      'include': [
        './www/detail.*',
        './www/index.*',
        './www/common.*',
        './www/flex.*',
        './www/vw.*'
      ],
      'ignore': [
        './**/*.md'
      ]
    },
    'support': {
      'ios': {
        'minSDK': '',
        'maxSDK': '',
        'minOS': '',
        'maxOS': ''
      },
      'android': {
        'minSDK': '',
        'maxSDK': '',
        'minOS': '',
        'maxOS': ''
      }
    }
  },
  'taobao': {
    'debug': taobaoSim,
    'deploy': 'awp',
    'container': 'taobao',
    'appid': pkg.appid,
    'name': pkg.name,
    'version': '1.0.0.1',
    'launchParams': {},
    'support': {},
    'build': {
      'include': [
        './www/flex.*',
        './www/vw.*',
        './www/index.*',
        './www/detail.*',
        './www/common.*'
      ],
      'ignore': [
        './**/*.md'
      ]
    }
  }
};
