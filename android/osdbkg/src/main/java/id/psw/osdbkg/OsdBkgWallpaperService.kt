package id.psw.osdbkg

import android.content.Context
import android.service.wallpaper.WallpaperService
import android.util.AttributeSet
import android.view.SurfaceHolder

class OsdBkgWallpaperService : WallpaperService() {
    private lateinit var renderer : OsdBkgSurfaceView.OsdBkgRenderer

    inner class WaveEngine(val ctx:Context) : Engine(){
        private lateinit var surface : OsdBkgSurface

        inner class OsdBkgSurface : OsdBkgSurfaceView {
            constructor(context: Context?): super(context){
            }
            constructor(context: Context?, attrs: AttributeSet): super(context, attrs){
            }
            constructor(context:Context?, attrs: AttributeSet, styleSet: Int) : super(context, attrs){
            }

            override fun getHolder(): SurfaceHolder {
                return surfaceHolder
            }
        }

        override fun onCreate(surfaceHolder: SurfaceHolder?) {
            super.onCreate(surfaceHolder)
            surface = OsdBkgSurface(ctx)
        }
    }

    override fun onCreateEngine(): Engine {
        return WaveEngine(this)
    }
}