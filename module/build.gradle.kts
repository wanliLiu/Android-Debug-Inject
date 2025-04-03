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
        from(project(":DebugInject").layout.buildDirectory.file("intermediates/cmake/$variantLowered/obj/"))
        into(moduleDir)

    }

    val zipTask = task<Zip>("zip$variantCapped") {
        group = "module"
        dependsOn(prepareModuleFilesTask)
        archiveFileName.set(zipFileName)
        destinationDirectory.set(layout.buildDirectory.file("outputs/release").get().asFile)
        from(moduleDir)
    }


}