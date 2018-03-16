package com.furture.litebrowser;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PathEffect;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.furture.litebrowser.common.BorderStyles;
import com.furture.litebrowser.common.Edge;
import com.furture.litebrowser.common.LayoutRect;
import com.furture.litebrowser.http.HttpClient;
import com.furture.litebrowser.http.UrlUtils;
import com.furture.litebrowser.img.ImageManager;
import com.furture.litebrowser.img.ImageRender;
import com.furture.litebrowser.jni.NativeUtils;
import com.furture.litebrowser.jni.StringPool;
import com.furture.litebrowser.thread.LiteThread;
import com.squareup.picasso.Picasso;

import java.io.UnsupportedEncodingException;

import static com.furture.litebrowser.common.Edge.BOTTOM;
import static com.furture.litebrowser.common.Edge.LEFT;
import static com.furture.litebrowser.common.Edge.RIGHT;
import static com.furture.litebrowser.common.Edge.TOP;

/**
 * Created by furture on 2018/3/9.
 */

public class LiteBrowser extends View implements Handler.Callback {

    private static final String TAG = "LiteBrowser";

    static {
        System.loadLibrary("litebrowser-lib");
    }

    private static  final  int MSG_LOAD_HTML = 0x01;
    private static  final  int MSG_LOAD_URL = 0x02;
    private static  final  int MSG_LAYOUT = 0x03;

    private static final  int STATE_INIT = 0;

    private static final  int STATE_HTML_PASS = 4;
    private static final  int STATE_RESIZE_LAYOUT = 6;
    private static final  int STATE_LAYOUT_PASS = 8;

    private long ptr;
    private Paint mBackgroundPaint;
    private Paint mBordersPaint;
    private Thread liteThread;
    private Looper liteLooper;
    private Handler liteHandler;
    private int state;
    private LayoutRect layoutRect;
    private LayoutRect preLayoutRect;
    private ImageManager imageManager;
    private String url;
    private Paint mBitmapPaint;
    private StringPool stringPool;

    public LiteBrowser(Context context) {
        super(context);
        initBrowser();
    }

    public LiteBrowser(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initBrowser();
    }

    public LiteBrowser(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initBrowser();
    }

    private void initBrowser(){
        String masterCss = AssetUtils.read(getContext(), "master.css");
        DisplayMetrics metrics  = getResources().getDisplayMetrics();
        this.ptr = init(masterCss, metrics.widthPixels, metrics.heightPixels,
                metrics.density, metrics.densityDpi, (int)(metrics.density*16));
        mBackgroundPaint = createNormalPaint();
        mBordersPaint = createNormalPaint();
        mBitmapPaint = createNormalPaint();
        mBordersPaint.setStyle(Paint.Style.STROKE);
        if(false){
            liteThread = Looper.getMainLooper().getThread();
            liteLooper = Looper.getMainLooper();
            liteHandler = new Handler(liteLooper,  this);
        }else{
            LiteThread thread = new LiteThread("LiteBrowsers");
            thread.start();
            liteThread = thread;
            liteLooper = thread.getLooper();
            liteHandler = new Handler(liteLooper,  this);
        }
        layoutRect = new LayoutRect();
        preLayoutRect = new LayoutRect();
        stringPool = new StringPool();
        imageManager = new ImageManager(this);
        state = STATE_INIT;
    }

    private Paint createNormalPaint(){
        DisplayMetrics metrics  = getResources().getDisplayMetrics();
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setTextSize((int)(metrics.density*16));
        return paint;
    }

    @Override
    protected void finalize() throws Throwable {
        destory();
        super.finalize();
    }

    public void destory(){
        if(ptr != 0){
            if(liteThread instanceof  LiteThread) {
                ((LiteThread)liteThread).quit();
                liteHandler.removeCallbacks(null);
            }
            destory(ptr);
            ptr = 0;
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        Log.e(TAG, "browser onMeasure " + widthMeasureSpec  + "  " + heightMeasureSpec);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        if(left > 0 || top > 0){
            Log.e(TAG, "browser onLayout padding not supported");
        }
        //if(layoutRect.rectEquals(left, top, right, bottom)) {
            layoutRect.left = left;
            layoutRect.top = top;
            layoutRect.right = right;
            layoutRect.bottom = bottom;
            Message msg = Message.obtain(liteHandler, MSG_LAYOUT, layoutRect);
            msg.sendToTarget();
        //}
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if(ptr != 0  && state >= STATE_LAYOUT_PASS){
            long start = System.currentTimeMillis();
            draw(ptr, canvas, 0, 0, getMeasuredWidth(), getMeasuredHeight());
            Log.e(TAG, "LiteView onDraw used " + (System.currentTimeMillis() - start));
        }
    }
    public void loadHtml(String html){
        this.url = null;
        stringPool.reset();
        if(Thread.currentThread() != liteThread){
            Message message = Message.obtain(liteHandler, MSG_LOAD_HTML, html);
            liteHandler.sendMessage(message);
        }else{
            loadHtmlString(html);
        }
    }
    public void loadUrl(String url){
        this.url = url;
        stringPool.reset();
        Message message = Message.obtain(liteHandler, MSG_LOAD_URL, url);
        liteHandler.sendMessage(message);

    }


    @Override
    public boolean handleMessage(Message msg) {
        switch (msg.what){
            case MSG_LOAD_HTML:{
                loadHtmlString((String) msg.obj);
            }
            break;
            case MSG_LOAD_URL:{
                try{
                    String html = HttpClient.get(getContext(), (String) msg.obj, null);
                    loadHtmlString(html);
                }catch (Exception e){
                    Log.e(TAG, "download url error ", e);
                    Toast.makeText(getContext(), e.getMessage() , Toast.LENGTH_SHORT).show();
                }
            }
            break;
            case MSG_LAYOUT:{
                if(ptr != 0  && state >= STATE_HTML_PASS){
                    if(state < STATE_LAYOUT_PASS){
                        if(!preLayoutRect.rectEquals(layoutRect)){
                            preLayoutRect.rectCopy(layoutRect);
                            //FIXME DOUBLE LAYOUT
                            int height = layout(ptr, preLayoutRect.right - preLayoutRect.left, preLayoutRect.bottom - preLayoutRect.top);
                            if(height > preLayoutRect.bottom - preLayoutRect.top && state != STATE_RESIZE_LAYOUT){
                                state = STATE_RESIZE_LAYOUT;
                                preLayoutRect.layoutHeight = height;
                                postRequestLayout(height);
                            }else{
                                state = STATE_LAYOUT_PASS;
                                postInvalidate();
                            }
                        }else{
                            if(state == STATE_RESIZE_LAYOUT){
                                state = STATE_LAYOUT_PASS;
                                postInvalidate();
                            }
                        }
                    }
                }
            }
            break;
            default:
                break;
        }
        return false;
    }

    private void loadHtmlString(String html){
        if(ptr != 0) {
            if(TextUtils.isEmpty(html)){
                return;
            }
            Log.e(TAG, html);
            loadHtml(ptr, html);
            if(Thread.currentThread() != Looper.getMainLooper().getThread()){
                postRequestLayout();
            }else{
                state = STATE_HTML_PASS;
            }
        }
    }


    private void postRequestLayout(){
        post(new Runnable() {
            @Override
            public void run() {
                state = STATE_HTML_PASS;
                requestLayout();
            }
        });
    }

    private void postRequestLayout(final int height){
        post(new Runnable() {
            @Override
            public void run() {
                ViewGroup.LayoutParams params = getLayoutParams();
                params.height = height;
                setLayoutParams(params);
            }
        });
    }

    /**
     * draw methods
     * */
    public Paint createPaint(String faceName, int size, int weight, int style, int decoration){
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setTextSize(size);
        Typeface typeface = Typeface.create(faceName, style);
        if(typeface == null){
            typeface = Typeface.defaultFromStyle(style);
        }
        paint.setTypeface(typeface);
        //paint.setStrokeWidth(size);
        return paint;
    }

    public int getXHeight(Paint paint){
        paint.getTextBounds("x", 0, 1, tempRect);
        return tempRect.height();
    }

    public int getTextWidth(Paint paint, long ptr){
        try {
            String string = stringPool.getString(ptr);
            return (int) (paint.measureText(string,0, string.length()) + 0.49);
        } catch (UnsupportedEncodingException e) {
            return (int) (paint.measureText("X") + 0.49f);
        }
    }

    public void drawText(Canvas canvas, Paint paint, long ptr, int x, int y, int width, int height){
        y = y + Math.abs(paint.getFontMetricsInt().ascent);
        try {
            canvas.drawText(stringPool.getString(ptr), x, y, paint);
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "drawText ", e);
        }
    }

    public void drawBackground(Canvas canvas, int color, String[] imgs, int x, int y, int width, int height,
                               int bottomLeftRadiusX, int bottomLeftRadiusY, int bottomRightRadiusX,int bottomRightRadiusY,
                               int topLeftRadiusX, int topLeftRadiusY, int topRightRadiusX,int topRightRadiusY,
                               String url, String baseUrl, int attachment, int repeat){

        //draw border
        if((color&0XFF000000) != 0){
            int oldColor = mBackgroundPaint.getColor();
            mBackgroundPaint.setColor(color);
            int right  = x + width;
            int bottom = y + height;
            if(topLeftRadiusX == 0
                    && topRightRadiusX == 0
                    && bottomLeftRadiusX == 0
                    && bottomRightRadiusX == 0){
                tempRectF.set(x, y,  right, bottom);
                canvas.drawRect(tempRectF, mBackgroundPaint);
            }else if(topLeftRadiusX == topRightRadiusX
                    && topRightRadiusX == bottomLeftRadiusX
                    && bottomLeftRadiusX == bottomRightRadiusX){
                tempRectF.set(x, y,  right, bottom);
                canvas.drawRoundRect(tempRectF, topLeftRadiusX, topLeftRadiusX, mBackgroundPaint);
            }else{
                Path path = new Path();
                path.moveTo(x, y  + topLeftRadiusX);
                if(topLeftRadiusX > 0){
                    tempRectF.set(x, y, x + topLeftRadiusX*2, y + topLeftRadiusX*2);
                     path.arcTo(tempRectF, 180, 90);
                 }
                 path.lineTo(right - topRightRadiusX , y);
                 if(topRightRadiusX > 0){
                    tempRectF.set(right - topRightRadiusX*2, y, right, y + topRightRadiusX*2);
                    path.arcTo(tempRectF, 270, 90);
                 }
                 path.lineTo(right, bottom - bottomRightRadiusX);
                 if(bottomRightRadiusX > 0){
                    tempRectF.set(right - bottomRightRadiusX*2,  bottom - bottomRightRadiusX*2, right, bottom);
                    path.arcTo(tempRectF, 0, 90);
                 }
                 path.lineTo(x  + bottomLeftRadiusX, bottom);
                 if(bottomLeftRadiusX > 0){
                     tempRectF.set(x,  bottom - bottomLeftRadiusX*2, x  + bottomLeftRadiusX*2, bottom);
                     path.arcTo(tempRectF, 90, 90);
                 }
                 path.close();
                 canvas.drawPath(path, mBackgroundPaint);
            }
            mBackgroundPaint.setColor(oldColor);
        }
        if(url == null){
            return;
        }
        //drawbackground
        ImageRender imageRender = imageManager.getBitmap(url, baseUrl);
        if(imageRender == null || imageRender.getBitmap() == null){
            return;
        }
        Rect src = new Rect();
        src.set(0, 0, imageRender.getBitmap().getWidth(), imageRender.getBitmap().getHeight());
        Rect dest = new Rect();
        dest.set(x, y, x + width, y + height);
        canvas.drawBitmap(imageRender.getBitmap(), src, dest, mBackgroundPaint);

        //fixme img supported
    }


    /**
     * choose start draw edge, then start draw clock direction
     * */
    public void drawBorders(Canvas canvas,
                            int x, int y, int width, int height,
                            int leftColor, int leftStyle, int leftWidth,
                            int rightColor, int rightStyle, int rightWidth,
                            int topColor, int topStyle, int topWidth,
                            int bottomColor, int bottomStyle, int bottomWidth,
                            int topLeftRadius, int topRightRadius, int bottomLeftRadius, int bottomRightRadius){

        int right  = x + width;
        int bottom = y + height;

        boolean save = false;
        /**
        if((topLeftRadius + bottomLeftRadius + leftWidth*2)  > height
                || (topLeftRadius + topRightRadius + topWidth*2)  > width
                || (topRightRadius + bottomRightRadius + rightWidth*2)  > height
                || (bottomLeftRadius + bottomRightRadius + bottomWidth*2)  > width){
            Path path = new Path();
            path.moveTo(x - topLeftRadius, y  + topLeftRadius);
            if(topLeftRadius > 0){
                tempRectF.set(x, y, x + topLeftRadius*2, y + topLeftRadius*2);
                path.arcTo(tempRectF, 180, 90);
            }
            path.lineTo(right - topRightRadius , y);
            if(topRightRadius > 0){
                tempRectF.set(right - topRightRadius*2, y, right, y + topRightRadius*2);
                path.arcTo(tempRectF, 270, 90);
            }
            path.lineTo(right, bottom - bottomRightRadius);
            if(bottomRightRadius > 0){
                tempRectF.set(right - bottomRightRadius*2,  bottom - bottomRightRadius*2, right, bottom);
                path.arcTo(tempRectF, 0, 90);
            }
            path.lineTo(x  + bottomLeftRadius, bottom);
            if(bottomLeftRadius > 0){
                tempRectF.set(x,  bottom - bottomLeftRadius*2, x  + bottomLeftRadius*2, bottom);
                path.arcTo(tempRectF, 90, 90);
            }
            path.close();
            canvas.save();
            canvas.clipPath(path);
            save = true;
        }*/


        /**
         * left top right bottom
         * */
        Edge edge = LEFT;
        if(leftColor != topColor
                || leftWidth != topWidth
                || leftStyle != topStyle){
            edge = TOP;
        }
        if(topColor != rightColor
                ||  topWidth != rightWidth
                ||  topStyle  != rightStyle){
            edge = RIGHT;
        }

        if(rightColor != bottomColor
                ||  rightWidth != bottomWidth
                ||  rightStyle  != bottomStyle){
            edge = BOTTOM;
        }

        if(bottomColor != leftColor
                || bottomWidth != leftWidth
                || bottomStyle != leftStyle){
            edge = LEFT;
        }
        //draw clock direction, four edge
        Path path = new Path();
        for(int i=0; i<4; i++){
            switch (edge){
                case LEFT:{
                    if(leftWidth > 0 && leftStyle > 1){
                        if(bottomLeftRadius > 0){
                            tempRectF.set(x, bottom - bottomLeftRadius * 2, x +bottomLeftRadius * 2, bottom);
                            path.arcTo(tempRectF, 135, 45, path.isEmpty());
                        }else {
                            path.moveTo(x, bottom - bottomLeftRadius);
                        }
                        if((topLeftRadius + bottomLeftRadius + leftWidth*2)  < height) {
                            path.lineTo(x, y + topLeftRadius);
                        }
                        if(topLeftRadius > 0){
                            tempRectF.set(x, y, x + topLeftRadius * 2,y + topLeftRadius * 2);
                            path.arcTo(tempRectF, 180, 45, false);
                        }
                        if(leftColor != topColor
                                || leftWidth != topWidth
                                || leftStyle != topStyle){
                            mBordersPaint.setColor(leftColor);
                            mBordersPaint.setStrokeWidth(leftWidth);
                            PathEffect pathEffect = mBordersPaint.getPathEffect();
                            mBordersPaint.setPathEffect(BorderStyles.getPathEffect(leftStyle, leftWidth));

                            canvas.drawPath(path, mBordersPaint);
                            mBordersPaint.setPathEffect(pathEffect);
                            path.reset();
                        }
                    }
                    edge = TOP;
                }
                break;
                case TOP:{
                    if(topWidth > 0 && topStyle > 1){
                        if(topLeftRadius > 0){
                            tempRectF.set(x, y, x + topLeftRadius * 2,y + topLeftRadius * 2);
                            path.arcTo(tempRectF, 225, 45, path.isEmpty());
                        }else{
                            path.moveTo(x + topLeftRadius, y);
                        }

                        if((topLeftRadius + topRightRadius + topWidth*2)  < width) {
                            path.lineTo(right - topRightRadius, y);
                        }

                        if(topRightRadius > 0){
                            tempRectF.set(right - topRightRadius*2, y, right,y + topRightRadius*2);
                            path.arcTo(tempRectF, 270, 45, false);
                        }
                        if(topColor != rightColor
                                ||  topWidth != rightWidth
                                ||  topStyle  != rightStyle){
                            mBordersPaint.setColor(topColor);
                            mBordersPaint.setStrokeWidth(topWidth);
                            PathEffect pathEffect = mBordersPaint.getPathEffect();
                            mBordersPaint.setPathEffect(BorderStyles.getPathEffect(topStyle, topWidth));
                            canvas.drawPath(path, mBordersPaint);
                            mBordersPaint.setPathEffect(pathEffect);
                            path.reset();
                        }
                    }
                    edge = RIGHT;
                }
                break;
                case RIGHT:{
                    if(rightWidth > 0 && rightStyle > 1){
                        if(topLeftRadius > 0){
                            tempRectF.set(right - topRightRadius*2, y, right,y + topRightRadius*2);
                            path.arcTo(tempRectF, 315, 45, path.isEmpty());
                        }else{
                            path.moveTo(right,  y + topLeftRadius);
                        }

                        if((topRightRadius + bottomRightRadius + rightWidth*2)  < height) {
                            path.lineTo(right, bottom - bottomRightRadius);
                        }

                        if(bottomRightRadius > 0){
                            tempRectF.set(right - bottomRightRadius*2, bottom - bottomRightRadius*2, right, bottom);
                            path.arcTo(tempRectF, 0, 45, false);
                        }
                        if(rightColor != bottomColor
                                ||  rightWidth != bottomWidth
                                ||  rightStyle  != bottomStyle){
                            mBordersPaint.setColor(rightColor);
                            mBordersPaint.setStrokeWidth(rightWidth);
                            PathEffect pathEffect = mBordersPaint.getPathEffect();
                            mBordersPaint.setPathEffect(BorderStyles.getPathEffect(rightStyle, rightWidth));
                            canvas.drawPath(path, mBordersPaint);
                            mBordersPaint.setPathEffect(pathEffect);
                            path.reset();
                        }
                    }
                    edge = BOTTOM;
                }
                break;
                case BOTTOM:{
                    if(bottomWidth > 0 && bottomStyle > 1){
                        if(bottomRightRadius > 0){
                            tempRectF.set(right - bottomRightRadius*2, bottom - bottomRightRadius*2, right, bottom);
                            path.arcTo(tempRectF, 45, 45, path.isEmpty());
                        }else{
                            path.moveTo(right - bottomRightRadius, bottom);
                        }

                        if((bottomLeftRadius + bottomRightRadius + bottomWidth*2)  < width) {
                            path.lineTo(x + bottomLeftRadius, bottom);
                        }

                        if(bottomLeftRadius > 0){
                            tempRectF.set(x, bottom - bottomLeftRadius * 2, x +bottomLeftRadius * 2, bottom);
                            path.arcTo(tempRectF, 90, 45, false);
                        }
                        if(bottomColor != leftColor
                                || bottomWidth != leftWidth
                                || bottomStyle != leftStyle){

                            mBordersPaint.setColor(bottomColor);
                            mBordersPaint.setStrokeWidth(bottomWidth);
                            PathEffect pathEffect = mBordersPaint.getPathEffect();
                            mBordersPaint.setPathEffect(BorderStyles.getPathEffect(bottomStyle, bottomWidth));
                            canvas.drawPath(path, mBordersPaint);
                            mBordersPaint.setPathEffect(pathEffect);
                            path.reset();
                        }
                    }
                    edge = LEFT;
                }
                break;
            }
        }
        //draw path continuesly and sequency
        if(!path.isEmpty()){
            switch (edge){
                case LEFT:{
                    mBordersPaint.setColor(leftColor);
                    mBordersPaint.setStrokeWidth(leftWidth);
                    PathEffect pathEffect = mBordersPaint.getPathEffect();
                    mBordersPaint.setPathEffect(BorderStyles.getPathEffect(leftStyle, leftWidth));

                    canvas.drawPath(path, mBordersPaint);
                    mBordersPaint.setPathEffect(pathEffect);
                    path.reset();
                }
                break;
                case TOP:{
                    mBordersPaint.setColor(topColor);
                    mBordersPaint.setStrokeWidth(topWidth);
                    PathEffect pathEffect = mBordersPaint.getPathEffect();
                    mBordersPaint.setPathEffect(BorderStyles.getPathEffect(topStyle, topWidth));
                    canvas.drawPath(path, mBordersPaint);
                    mBordersPaint.setPathEffect(pathEffect);
                    path.reset();
                }
                break;
                case RIGHT:{
                    mBordersPaint.setColor(rightColor);
                    mBordersPaint.setStrokeWidth(rightWidth);
                    PathEffect pathEffect = mBordersPaint.getPathEffect();
                    mBordersPaint.setPathEffect(BorderStyles.getPathEffect(rightStyle, rightWidth));
                    canvas.drawPath(path, mBordersPaint);
                    mBordersPaint.setPathEffect(pathEffect);
                    path.reset();
                }
                break;
                case BOTTOM:{
                    mBordersPaint.setColor(bottomColor);
                    mBordersPaint.setStrokeWidth(bottomWidth);
                    PathEffect pathEffect = mBordersPaint.getPathEffect();
                    mBordersPaint.setPathEffect(BorderStyles.getPathEffect(bottomStyle, bottomWidth));
                    canvas.drawPath(path, mBordersPaint);
                    mBordersPaint.setPathEffect(pathEffect);
                    path.reset();
                }
                break;

            }
        }
        if(save){
            canvas.restore();
        }
    }

    /**
     * import css sync, should be called on background thread
     * */
    public String importCss(String url, String baseUrl){
        try {
            if(baseUrl == null){
                baseUrl = this.url;
            }
            String css =  HttpClient.get(getContext(), url, baseUrl);
            Log.e(TAG, css);
            return css;
        } catch (Exception e) {
            Log.e(TAG, "importCss " + url, e);
            return null;
        }
    }

    public void loadImg(String url, String baseUrl,boolean redraw_on_ready){
        imageManager.loadImg(url, baseUrl, redraw_on_ready);
    }

    public void getImageSize(String url, String baseUrl, long ptr){
       imageManager.getImageSize(url, baseUrl, ptr);
    }


    private Rect tempRect = new Rect();
    private RectF tempRectF = new RectF();

    private native long  init(String masterCss, int screenWidth,
                              int screenHeight, float density, int densityDpi, int defaultFontSize);
    private native int  layout(long ptr, int width, int height);
    private native void  loadHtml(long ptr, String html);
    private native void  draw(long ptr, Canvas canvas, int left, int right, int width, int height);
    private native void  destory(long ptr);


    private static final Canvas DUMMY_CANVAS = new Canvas();
}
