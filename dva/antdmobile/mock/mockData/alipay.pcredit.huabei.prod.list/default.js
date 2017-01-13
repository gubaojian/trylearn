export default function (viewId) {
  const data =  {
  success: true,
  result: {
    total: "1,2324.00",
    totalBuy: "8345.00",
    totalSell: "6234.00",
    totalReward: "234.00",
    records: [
      {
        productName: '活期宝2号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '1000.00', // 金额
        },
        orderStatus: 'BUY', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '100.00', // 金额
        },
        orderStatus: 'BUY', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '100.00', // 金额
        },
        orderStatus: 'SELL', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '1003.00', // 金额
        },
        orderStatus: 'SELL', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '5000.00', // 金额
        },
        orderStatus: 'REWARD', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '19000.00', // 金额
        },
        orderStatus: 'REWARD', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '1100.00', // 金额
        },
        orderStatus: 'SELL', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '12000.00', // 金额
        },
        orderStatus: 'REWARD', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
      {
        productName: '活期宝1号', // 产品名称
        payTime: 1481039360180, // 付款时间
        applyAmount: {
          cent: '14000.00', // 金额
        },
        orderStatus: 'SELL', // 订单状态
        orderId: 6283749809128436957924985093, // 订单id
      },
    ],
  },
};

console.log(viewId);
  const mock = {
    default: data,
    'error': {
      success: false,
      data: {
        name: 'anima-new'
      }
    },
    'index-old': data,
    'apply-new': data
  };

  return mock[viewId] || mock.default;
}
