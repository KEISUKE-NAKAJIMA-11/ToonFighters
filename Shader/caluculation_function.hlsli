//rambert shder
float3 Diffuse(float3 N, float3 L, float3 C, float3 K)
{
    float D = dot(N, -L);
    D = max(0, D); // •‰‚Ì’l‚ğ‚O‚É‚·‚é
    return K * C * D;
}

float3 HalfLambart(float3 N, float3 L, float3 C, float3 K, float A)
{
    float D = dot(N, L) * 0.5 + 0.5;
    D *= D;
    return (K * C) * (D * A);
}
float3 HalfLambart_L(float3 N, float3 L, float3 C, float3 K)
{
    float D = dot(N, L) * 0.5 + 0.5;
    D *= D;
    return (K * C) * D;
}

//blinnphong shader
float3 BlinnPhongSpecular(float3 N, float3 L, float3 C, float3 E,
	float3 K, float Power)
{
	//ƒn[ƒtƒxƒNƒgƒ‹
    float3 H = normalize(-L + E);

    float3 S = dot(H, N);
    S = max(0, S);
    S = pow(S, Power);
    S = S * K * C;
    return S;
}

//phong shader
float3 PhongSpecular(float3 N, float3 L, float3 C, float3 E,
	float3 K, float Power)
{
    float3 R = reflect(L, N);
    R = normalize(R);
    float3 S = dot(R, E);
    S = max(0, S);
    S = pow(S, Power);
    S = S * K * C;
    return S;
}



float3 HemiSphereLight(float3 N, float3 SkyColor, float3 GroundColor)
{
    float skyblend = (N.y + 1.0) * 0.5;
    float groundblend = 1.0 - skyblend;
    return SkyColor * skyblend + GroundColor * groundblend;
}

//toon shader
//‚RŠK’²
float3 ToonLight(float3 N, float3 L, float3 C, float3 K)
{
    float D = dot(N, -L);
    if (D < 0.3)
        D = 0.2;
    else if (D < 0.9)
        D = 0.6;
    else
        D = 1.4;


    return K * C * D;
}


//‚QŠK’²
float3 ToonDiffuse(float3 N, float3 L, float3 C, float3 K)
{
    float D = dot(N, -L);
    if (D < 0.3)
        D = 0.2;
    else
        D = 0.6;


    return K * C * D;
}

float3 ToonSpecular(float3 N, float3 L, float3 C, float3 E,
	float3 K, float Power)
{
    float3 R = reflect(L, N);
    R = normalize(R);
    float S = dot(R, E);
    S = max(0, S);
    S = pow(S, Power);
    if (S > 0.5)
        S = 0.8;
    else
        S = 0.4;
    return S * K * C;
}

