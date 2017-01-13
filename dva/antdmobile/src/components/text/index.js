import React, { Component } from 'react';

import styles from './style/index.less';

class Text extends Component {
  render() {
    return (
      <p className={styles.enhance}>
        {this.props.children}
      </p>
    );
  }
}

export default Text;
