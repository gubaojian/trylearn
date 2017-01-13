import vw from '@ali/anima-hd/lib/vw.js';
import flex from '@ali/anima-hd/lib/flex.js';

const doc = document;
const docEl = doc.documentElement;
const clientWidth = docEl.clientWidth;
if (clientWidth >= 750) {
  vw(100, 750);
} else {
  flex();
}
