package id.psw.osdbkg

import android.app.Activity
import android.app.WallpaperManager
import android.os.Bundle
import android.os.PersistableBundle

class GLESActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(OsdBkgSurfaceView(this))
        actionBar?.hide()
    }
}