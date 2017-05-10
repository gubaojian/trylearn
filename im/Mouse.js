class Mouse {
    constructor(options){
        let defaultOptions = {
            dragLength: 0,
        };
        this.options = mergeObjects(defaultOptions,options);

        this._startX = 0;
        this._x = 0;
        this._isDown = false;
        this._isDragging = false;
    }

    isBetween(left, right){
        return (this._x >= left) && (right >= this._x);
    }
}
export default Mouse;
