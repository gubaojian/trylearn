import Tracker from '@alipay/mtracker/lib/seed';

const Lj = store => next => action => {
  const lj = action.lengjing;
  if (lj) {
    if (lj.seedName) {
      Tracker.click(lj.seedName, lj.extra || {});
    } else {
      typeof console !== 'undefined' && console.warn && console.warn('棱镜埋点时,seedName不得为空 \n', store.getState());
    }
  }
  next(action);
};

export default Lj;
