package id.psw.osdbkg

import android.content.Context
import android.graphics.Canvas
import android.graphics.PixelFormat
import android.opengl.GLSurfaceView
import android.os.SystemClock
import android.util.AttributeSet
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

open class OsdBkgSurfaceView : GLSurfaceView {
    inner class OsdBkgRenderer : GLSurfaceView.Renderer {
        private var lastTime = 0L

        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            LibOsdBkg.start()
            lastTime = SystemClock.elapsedRealtime()
        }

        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            LibOsdBkg.resize(width, height)
        }

        override fun onDrawFrame(gl: GL10?) {
            val currentTime = SystemClock.elapsedRealtime()
            val deltaTime = currentTime - lastTime
            lastTime = currentTime;
            LibOsdBkg.draw(deltaTime * 0.001f)
        }
    }

    constructor(context: Context?): super(context){
        init()
    }
    constructor(context: Context?, attrs: AttributeSet): super(context, attrs){
        init()
    }

    constructor(context:Context?, attrs: AttributeSet, styleSet: Int) : super(context, attrs){
        init()
    }

    private lateinit var renderer : OsdBkgRenderer
    fun init(){
        setEGLConfigChooser(8,8,8,0,0,0)
        setEGLContextClientVersion(3)
        renderer = OsdBkgRenderer()
        setRenderer(renderer)
        renderMode = RENDERMODE_CONTINUOUSLY
        holder.setFormat(PixelFormat.TRANSLUCENT)
    }

    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas)
    }
}