package com.furture.litebrowser.common;

/**
 * Created by furture on 2018/3/15.
 */

public class LayoutRect {
    public int left;
    public int top;
    public int right;
    public int bottom;

    public int layoutHeight;

    public boolean rectEquals(LayoutRect that) {
        if (left != that.left) return false;
        if (top != that.top) return false;
        if (right != that.right) return false;
        return bottom == that.bottom;
    }

    public void rectCopy(LayoutRect that) {
        this.left = that.left;
        this.top = that.top;
        this.right = that.right;
        this.bottom = that.bottom;
    }

    public boolean rectEquals(int left, int top, int right, int bottom) {
        if (left != this.left) return false;
        if (top != this.top) return false;
        if (right != this.right) return false;
        return bottom == this.bottom;
    }
}
