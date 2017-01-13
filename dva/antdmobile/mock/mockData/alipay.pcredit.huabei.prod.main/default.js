export default function (viewId) {
  const data = {
    success: true,
    result: {
      name: 'anima'
    }
  };

  const mock = {
    default: data,
    'index-new': {
      success: true,
      data: {
        name: 'anima-new'
      }
    },
    'index-old': data,
    'apply-new': data
  };

  return mock[viewId] || mock.default;
}
