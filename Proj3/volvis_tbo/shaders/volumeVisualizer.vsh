#version 410 core

in vec3 mcPosition;
in vec3 mcNormal;
uniform mat4 mc_ec, ec_lds;

out PVA
{
	// Use this if you are ray tracing voxel grid in fragment shader
	vec3 mcPosition;
	// Use this for checking that voxel geometry size looks ok
	vec3 mcNormal;
} pvaOut;

void main()
{
	pvaOut.mcPosition = mcPosition;
	pvaOut.mcNormal = mcNormal;

	gl_Position = ec_lds *  mc_ec * vec4(mcPosition, 1.0);
}

