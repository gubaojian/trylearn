import React from 'react'
import styles from './style/index.less';



class Panel extends React.Component {
  render(){
    var props = this.props;
    return (<div className={styles.panel}>
             <div className={styles['panel-title']}>{this.props.title}</div>
             <div className={styles['panel-amount']}>{this.props.amount}</div>
           </div>);
  }
}

export default Panel;
