import React from 'react'
import {Button} from 'antd'


const MYButton =(props)=>{
  return (
    <div>
      <Button onClick={props.onClick}>点击这里{props.text}</Button>
      <div>{props.data}</div>
    </div>
  )
};

MYButton.propTypes ={

}

export default MYButton;
