import android.databinding.tool.ext.capitalizeUS


plugins {
    alias(libs.plugins.android.application)
}
android{
    namespace = "icu.nullptr.zygisk.next"
    compileSdkVersion = "android-34"
    buildFeatures {
        resValues = false
        buildConfig = false

    }
}

val moduleId: String by rootProject.extra
val moduleName: String by rootProject.extra
val verName: String by rootProject.extra


androidComponents.onVariants { variant ->
    val variantLowered = variant.name.lowercase()
    val variantCapped = variant.name.capitalizeUS()
    val buildTypeLowered = variant.buildType?.lowercase()
    val moduleDir = layout.buildDirectory.dir("outputs/module/$variantLowered")
    val zipFileName = "text-$buildTypeLowered.zip".replace(' ', '-')
    val prepareModuleFilesTask = task<Sync>("prepareModuleFiles$variantCapped") {
        println(variantCapped)

        dependsOn(
            ":DebugInject:externalNativeBuild$variantCapped",
            ":Zygisk:externalNativeBuild$variantCapped",
        )
        into(moduleDir)
        from("$projectDir/src") {
            exclude("module.prop")
//            filter<FixCrLfFilter>("eol" to FixCrLfFilter.CrLf.newInstance("lf"))
        }
        from("$projectDir/src") {
            include("module.prop")
            expand(
                "moduleId" to moduleId,
                "moduleName" to moduleName,
                "versionName" to "$verName ($variantLowered)",
                "versionCode" to "01"
            )
        }

        into("lib/arm64-v8a"){
            from(project(":Zygisk").layout.buildDirectory.file("intermediates/cmake/$variantLowered/obj/arm64-v8a/libzygisk.so"))
        }
        into("bin"){
            from(project(":Zygisk").layout.buildDirectory.file("intermediates/cmake/$variantLowered/obj/arm64-v8a/zygiskd"))
            from(project(":DebugInject").layout.buildDirectory.file("intermediates/cmake/$variantLowered/obj/arm64-v8a/ptraceInit"))

        }


    }

    val zipTask = task<Zip>("zip$variantCapped") {
        group = "module"
        dependsOn(prepareModuleFilesTask)
        archiveFileName.set(zipFileName)
        destinationDirectory.set(layout.buildDirectory.file("outputs/release").get().asFile)
        from(moduleDir)
    }


}