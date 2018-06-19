#version 130

//#define NPR
//#define SILHOUETTE

in vec3 fN;
in vec3 fV;
in vec3 fL; // Light Direction 在鏡頭座標下的方向
in vec3 fL2;
in vec3 fL3;
in vec3 fL4;

// 以下為新增的部分
uniform float fShininess;
uniform int   iLighting;
uniform vec4  vObjectColor;    // 代表物件的單一顏色

// Light 1
uniform vec4  LightInView;        // Light's position in View Space
uniform vec4  AmbientProduct;	// light's ambient  與 Object's ambient  與 ka 的乘積
uniform vec4  DiffuseProduct;	// light's diffuse  與 Object's diffuse  與 kd 的乘積
uniform vec4  SpecularProduct;	// light's specular 與 Object's specular 與 ks 的乘積

// Light 2
uniform vec4  LightInView2;        // Light's position in View Space
uniform vec4  AmbientProduct2;  // light's ambient  與 Object's ambient  與 ka 的乘積
uniform vec4  DiffuseProduct2;  // light's diffuse  與 Object's diffuse  與 kd 的乘積
uniform vec4  SpecularProduct2; // light's specular 與 Object's specular 與 ks 的乘積

// Light 3
uniform vec4  LightInView3;        // Light's position in View Space
uniform vec4  AmbientProduct3;  // light's ambient  與 Object's ambient  與 ka 的乘積
uniform vec4  DiffuseProduct3;  // light's diffuse  與 Object's diffuse  與 kd 的乘積
uniform vec4  SpecularProduct3; // light's specular 與 Object's specular 與 ks 的乘積

// Light 4
uniform vec4  LightInView4;        // Light's position in View Space
uniform vec4  AmbientProduct4;  // light's ambient  與 Object's ambient  與 ka 的乘積
uniform vec4  DiffuseProduct4;  // light's diffuse  與 Object's diffuse  與 kd 的乘積
uniform vec4  SpecularProduct4; // light's specular 與 Object's specular 與 ks 的乘積


void main()
{
	// 先宣告 diffuse 與 specular
    vec4 diffuse = vec4(0.0,0.0,0.0,1.0);
	vec4 specular = vec4(0.0,0.0,0.0,1.0);

	vec4 diffuse2 = vec4(0.0,0.0,0.0,1.0);
	vec4 specular2 = vec4(0.0,0.0,0.0,1.0);

	vec4 diffuse3 = vec4(0.0,0.0,0.0,1.0);
	vec4 specular3 = vec4(0.0,0.0,0.0,1.0);

	vec4 diffuse4 = vec4(0.0,0.0,0.0,1.0);
	vec4 specular4 = vec4(0.0,0.0,0.0,1.0);
	
	if( iLighting != 1 ) {
		gl_FragColor = vObjectColor;
	}
	else {	
		// 1. 計算 Ambient color : Ia = AmbientProduct = Ka * Material.ambient * La = 
		vec4 ambient = AmbientProduct; // m_sMaterial.ka * m_sMaterial.ambient * vLightI;

		// 單位化傳入的 Normal Dir
		vec3 vN = normalize(fN); 

		// 2. 單位化傳入的 Light Dir
		vec3 vL = normalize(fL); // normalize light vector
		vec3 vL2 = normalize(fL2); // normalize light vector
		vec3 vL3 = normalize(fL3); // normalize light vector
		vec3 vL4 = normalize(fL4); // normalize light vector

		// 5. 計算 L dot N
		float fLdotN = vL.x*vN.x + vL.y*vN.y + vL.z*vN.z;				//------ Light 1 ------//
		if( fLdotN >= 0 ) { // 該點被光源照到才需要計算
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse = fLdotN * DiffuseProduct; 
#else
			if( fLdotN >= 0.85 ) diffuse = 1.0 * DiffuseProduct;
			else if( fLdotN >= 0.55 ) diffuse = 0.55 * DiffuseProduct;
			else diffuse = 0.35 * DiffuseProduct;
#endif

			// Specular color
			// Method 1: Phone Model
			// 計算 View Vector
			// 單位化傳入的 fV , View Direction
			vec3 vV = normalize(fV);

			//計算 Light 的 反射角 vRefL = 2 * fLdotN * vN - L
			// 同時利用 normalize 轉成單位向量
			vec3 vRefL = normalize(2.0f * (fLdotN) * vN - vL);

			//   計算  vReflectedL dot View
			float RdotV = vRefL.x*vV.x + vRefL.y*vV.y + vRefL.z*vV.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV > 0 ) specular = SpecularProduct * pow(RdotV, fShininess); 
#else
			specular = vec4(0.0,0.0,0.0,1.0); 
#endif
		}
		

		float fLdotN2 = vL2.x*vN.x + vL2.y*vN.y + vL2.z*vN.z;				//------ Light 2 ------//
		if( fLdotN2 >= 0 ) { // 該點被光源照到才需要計算
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse2 = fLdotN2 * DiffuseProduct2; 
#else
			if( fLdotN2 >= 0.85 ) diffuse2 = 1.0 * DiffuseProduct2;
			else if( fLdotN2 >= 0.55 ) diffuse2 = 0.55 * DiffuseProduct2;
			else diffuse2 = 0.35 * DiffuseProduct2;
#endif

			// Specular color
			// Method 1: Phone Model
			// 計算 View Vector
			// 單位化傳入的 fV , View Direction
			vec3 vV2 = normalize(fV);

			//計算 Light 的 反射角 vRefL = 2 * fLdotN * vN - L
			// 同時利用 normalize 轉成單位向量
			vec3 vRefL2 = normalize(2.0f * (fLdotN2) * vN - vL2);

			//   計算  vReflectedL dot View
			float RdotV2 = vRefL2.x*vV2.x + vRefL2.y*vV2.y + vRefL2.z*vV2.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV2 > 0 ) specular2 = SpecularProduct2 * pow(RdotV2, fShininess); 
#else
			specular2 = vec4(0.0,0.0,0.0,1.0); 
#endif
		}

		float fLdotN3 = vL3.x*vN.x + vL3.y*vN.y + vL3.z*vN.z;				//------ Light 3 ------//
		if( fLdotN3 >= 0 ) { // 該點被光源照到才需要計算
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse3 = fLdotN3 * DiffuseProduct3; 
#else
			if( fLdotN3 >= 0.85 ) diffuse3 = 1.0 * DiffuseProduct3;
			else if( fLdotN3 >= 0.55 ) diffuse3 = 0.55 * DiffuseProduct3;
			else diffuse3 = 0.35 * DiffuseProduct3;
#endif

			// Specular color
			// Method 1: Phone Model
			// 計算 View Vector
			// 單位化傳入的 fV , View Direction
			vec3 vV3 = normalize(fV);

			//計算 Light 的 反射角 vRefL = 2 * fLdotN * vN - L
			// 同時利用 normalize 轉成單位向量
			vec3 vRefL3 = normalize(2.0f * (fLdotN3) * vN - vL3);

			//   計算  vReflectedL dot View
			float RdotV3 = vRefL3.x*vV3.x + vRefL3.y*vV3.y + vRefL3.z*vV3.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV3 > 0 ) specular3 = SpecularProduct3 * pow(RdotV3, fShininess); 
#else
			specular3 = vec4(0.0,0.0,0.0,1.0); 
#endif
		}

		float fLdotN4 = vL4.x*vN.x + vL4.y*vN.y + vL4.z*vN.z;				//------ Light 4 ------//
		if( fLdotN4 >= 0 ) { // 該點被光源照到才需要計算
#ifndef NPR
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse4 = fLdotN4 * DiffuseProduct4; 
#else
			if( fLdotN4 >= 0.85 ) diffuse4 = 1.0 * DiffuseProduct4;
			else if( fLdotN4 >= 0.55 ) diffuse4 = 0.55 * DiffuseProduct4;
			else diffuse4 = 0.35 * DiffuseProduct4;
#endif

			// Specular color
			// Method 1: Phone Model
			// 計算 View Vector
			// 單位化傳入的 fV , View Direction
			vec3 vV4 = normalize(fV);

			//計算 Light 的 反射角 vRefL = 2 * fLdotN * vN - L
			// 同時利用 normalize 轉成單位向量
			vec3 vRefL4 = normalize(2.0f * (fLdotN4) * vN - vL4);

			//   計算  vReflectedL dot View
			float RdotV4 = vRefL4.x*vV4.x + vRefL4.y*vV4.y + vRefL4.z*vV4.z;

#ifndef NPR
			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV4 > 0 ) specular4 = SpecularProduct4 * pow(RdotV4, fShininess); 
#else
			specular4 = vec4(0.0,0.0,0.0,1.0); 
#endif
		}


		// ------------ 計算顏色 ambient + diffuse + specular -------------
		gl_FragColor = ambient + diffuse + specular + diffuse2 + specular2 + diffuse3 + specular3 + diffuse4 + specular4;  

		// 設定 alpha
		// 設定為傳入材質的 alpha,DiffuseProduct.w
		gl_FragColor.w = DiffuseProduct.w;
		// gl_FragColor = vec4((ambient + diffuse + specular).xyz, 1.0);

#ifdef SILHOUETTE
	vec4 edgeColor = vec4(1.0, 0.0, 0.0, 1.0);
	if( abs(dot(normalize(fN), normalize(fV))) < 0.2)  gl_FragColor = edgeColor;
#endif

	}
}
