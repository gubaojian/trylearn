import React, {Component} from 'react';
import {findDOMNode} from 'react-dom';

const lengjingWrapper = function(WrappedComponent, data) {
  return class LengjingWrapper extends Component {

    componentDidMount() {

      if (data) {
        const el = findDOMNode(this);

        data.seedName && el.setAttribute('data-seed', data.seedName);
        data.expo && el.setAttribute('data-expo', data.expo);

        if (data.extra && Object.prototype.toString.call(data.extra) === '[object Object]') {
          for (let key in data.extra) {
            el.setAttribute(`data-mtr-${key}`, data.extra[key]);
          }
        }
      }
    }

    render() {
      return <WrappedComponent {...this.props} />;
    }
  };
};

export default lengjingWrapper;
