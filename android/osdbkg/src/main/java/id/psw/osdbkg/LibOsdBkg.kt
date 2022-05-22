package id.psw.osdbkg

object LibOsdBkg {
    init{
        System.loadLibrary("osdbkg")
    }
    external fun start()
    external fun draw(ms:Float)
    external fun resize(w:Int, h:Int)
}