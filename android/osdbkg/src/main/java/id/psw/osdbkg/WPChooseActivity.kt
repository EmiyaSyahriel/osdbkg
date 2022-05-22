package id.psw.osdbkg

import android.app.Activity
import android.app.WallpaperManager
import android.content.ComponentName
import android.content.Intent

class WPChooseActivity : Activity() {
    override fun onStart(){
        super.onStart()

        Intent().apply {
            action = WallpaperManager.ACTION_CHANGE_LIVE_WALLPAPER
            putExtra(
                WallpaperManager.EXTRA_LIVE_WALLPAPER_COMPONENT,
                ComponentName(
                    OsdBkgWallpaperService::class.java.`package`?.name ?: "id.psw.osdbkg",
                    OsdBkgWallpaperService::class.java.canonicalName ?: "OsdBkgWallpaperService"
                )
            )

        }.also {
            startActivity(it)
        }

        finish()
    }
}