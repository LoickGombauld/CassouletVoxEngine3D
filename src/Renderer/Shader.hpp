#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Voxel
{
	class Shader
	{
	public:

		Shader(
			const std::string& vertexPath,
			const std::string& fragmentPath
		);

		~Shader();

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		void bind() const;
		void unbind() const;

		unsigned int getID() const;

		void setMat4(const char* name, const glm::mat4& matrix) const;

		void setInt(const char* name, int value) const;

		void setFloat(const char* name, float value) const;

	private:

		unsigned int compileShader(
			unsigned int type,
			const std::string& source
		);

		unsigned int createProgram(
			unsigned int vertexShader,
			unsigned int fragmentShader
		);

		std::string readFile(
			const std::string& path
		);

		// Retourne l'emplacement d'un uniform en le mettant en cache,
		// afin d'éviter un appel glGetUniformLocation (recherche par
		// nom côté driver) à chaque frame et par chunk rendu.
		int getUniformLocation(
			const char* name
		) const;

	private:

		unsigned int m_programID = 0;

		mutable std::unordered_map<std::string, int> m_uniformLocationCache;
	};
}