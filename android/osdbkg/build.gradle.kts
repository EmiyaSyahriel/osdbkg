import org.jetbrains.kotlin.config.KotlinCompilerVersion

plugins {
    id("com.android.application")
    id("kotlin-android")
    kotlin("android")
}

android {
    compileSdk=31

    aaptOptions.cruncherEnabled = false

    defaultConfig {
        applicationId = "id.psw.osdbkg"
        minSdk = 19
        targetSdk = 31
        versionCode = 1
        versionName = "1.0"

        ndk {
            abiFilters.clear()
            abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64"))
        }
        // testInstrumentationRunner "androidx.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                // cppFlags = ""
            }
        }
    }

    buildTypes{
        getByName("release"){
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    kotlinOptions {
        jvmTarget = JavaVersion.VERSION_1_8.toString()
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.18.1"
        }
    }
}

dependencies {
}