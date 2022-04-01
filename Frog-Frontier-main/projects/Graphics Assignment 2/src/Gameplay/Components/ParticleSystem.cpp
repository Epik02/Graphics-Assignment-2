#include "ParticleSystem.h"
#include "Utils/JsonGlmHelpers.h"
#include "Application/Timing.h"
#include "Application/Application.h"
#include "Utils/ImGuiHelper.h"

ParticleSystem::ParticleSystem() :
	IComponent(),
	_hasInit(false),
	_maxParticles(1000),
	_numParticles(0),
	_particleBuffers(),
	_feedbackBuffers(),
	_query(0),
	_currentVertexBuffer(0),
	_currentFeedbackBuffer(1),
	_updateShader(nullptr),
	_renderShader(nullptr),
	_gravity({ 0, 0, -9.81f }),
	_emitters()
{ }

ParticleSystem::~ParticleSystem()
{
	if (_hasInit) {
		glDeleteBuffers(2, _particleBuffers);
		glDeleteTransformFeedbacks(2, _feedbackBuffers);
		glDeleteQueries(1, &_query);
		_updateShader = nullptr;
		_renderShader = nullptr;
	}
}

void ParticleSystem::Update()
{
	// If we haven't previously initialized our data, initialize it now
	if (!_hasInit) {
		// Allocate some temp space for particles, so we can init the emitters
		size_t dataSize = (_maxParticles + _emitters.size()) * sizeof(ParticleData);
		ParticleData* data = new ParticleData[_maxParticles + _emitters.size()];
		memset(data, 0, dataSize);

		// Add all emitter to the the particle list at the beginning
		for (int ix = 0; ix < _emitters.size(); ix++) {
			data[ix] = _emitters[ix];
		}

		// We essentially use double buffering, hence the 2 buffers
		glCreateTransformFeedbacks(2, _feedbackBuffers);
		glCreateBuffers(2, _particleBuffers);

		// Set up our first transform feedback buffer to write to the first buffer
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _feedbackBuffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, _particleBuffers[0]);
		glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _particleBuffers[0]);

		// Set up the second transform feedback buffer to write to the second buffer
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _feedbackBuffers[1]);
		glBindBuffer(GL_ARRAY_BUFFER, _particleBuffers[1]);
		glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _particleBuffers[1]);

		// We create a query object to track the number of particles we're simulating
		glGenQueries(1, &_query);

		// We no longer need the CPU copy
		delete[] data;
	}


	// Disable rasterization, this is update only
	glEnable(GL_RASTERIZER_DISCARD);

	// Make sure no VAOs are bound
	glBindVertexArray(0);

	// Bind the buffer and transform feedback
	glBindBuffer(GL_ARRAY_BUFFER, _particleBuffers[_currentVertexBuffer]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _feedbackBuffers[_currentFeedbackBuffer]);

	// Enable our attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4); 
	glEnableVertexAttribArray(5);

	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(ParticleData), 0); // type
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*)offsetof(ParticleData, Position)); // position
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*)offsetof(ParticleData, Velocity)); // velocity
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*)offsetof(ParticleData, Color)); // color 
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*)offsetof(ParticleData, Lifetime)); // metadata 
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*)offsetof(ParticleData, Metadata)); // metadata 

	// Bind the update shader and send our relevant uniforms
	_updateShader->Bind();
	_updateShader->SetUniform("u_Gravity", _gravity); 
	_updateShader->SetUniformMatrix("u_ModelMatrix", GetGameObject()->GetTransform()); 

	// Our particles are points that we're simulating
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, _query);
	glBeginTransformFeedback(GL_POINTS);

	// If this is our first pass, we use drawArrays to get the initial state, otherwise we use transform feedback for rendering
	if (!_hasInit) {
		glDrawArrays(GL_POINTS, 0, _emitters.size());
	}
	else {
		glDrawTransformFeedback(GL_POINTS, _feedbackBuffers[_currentVertexBuffer]);
	}

	// End of transform feedback
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	// Use our query to get the number of particles
	glGetQueryObjectuiv(_query, GL_QUERY_RESULT, &_numParticles);
	if (_numParticles >= _emitters.size()) {
		_numParticles -= _emitters.size();
	}
	else {
		_numParticles = 0;
	}

	// Clean up our state
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(5);

	// Re-enable rasterization for later OpenGL calls
	glDisable(GL_RASTERIZER_DISCARD);

	_hasInit = true;

	// Double-buffering, swap which buffers we're operating on
	_currentVertexBuffer = _currentFeedbackBuffer;
	_currentFeedbackBuffer = (_currentFeedbackBuffer + 1) & 0x01;
}

void ParticleSystem::Render()
{
	// Make sure that we've actually initialized our stuff
	if (_hasInit) {

		// We're using our particle rendering shader
		_renderShader->Bind();

		// Make sure no VAOs are bound
		glBindVertexArray(0);

		glEnablei(GL_BLEND, 0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Bind the current feedback buffer as our drawing buffer
		glBindBuffer(GL_ARRAY_BUFFER, _particleBuffers[_currentVertexBuffer]); 

		// Enable type, position and color 
		glEnableVertexAttribArray(0); 
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(ParticleData), 0); // type
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*)offsetof(ParticleData, Position)); // position
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*)offsetof(ParticleData, Color)); // color 

		// Draw our particles using whatever data we have in transform feedback buffer
		glDrawTransformFeedback(GL_POINTS, _feedbackBuffers[_currentVertexBuffer]);

		// Clean up after ourselves
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(3);
	}
}

void ParticleSystem::AddEmitter(const glm::vec3& position, const glm::vec3& direction, float emitRate /*= 1.0f*/, const glm::vec4& color /*= glm::vec4(1.0f)*/)
{
	LOG_ASSERT(!_hasInit, "Cannot add an emitter after the particle system has been initialized");

	ParticleData emitter;
	emitter.Type     = ParticleType::Emitter; 
	emitter.Position = position; 
	emitter.Velocity = direction;
	emitter.Lifetime = 1.0f / emitRate; 
	emitter.Color    = color;
	emitter.Metadata = { 1.0f / emitRate, 0.0f, 2.0f, 4.0f };

	_emitters.push_back(emitter); 
}

void ParticleSystem::RenderImGui()
{
	LABEL_LEFT(ImGui::LabelText, "Particle Count", "%u", _numParticles);

	Application& app = Application::Get();

	ImGui::Separator();
	ImGui::Text("Emitters:");

	// We can't add or edit emitters once the system has started
	if (!app.CurrentScene()->IsPlaying) {
		for (int ix = 0; ix < _emitters.size(); ix++) {
			auto& emitter = _emitters[ix];

			ImGui::PushID(&emitter);
			if (ImGui::CollapsingHeader("Emitter")) {
				LABEL_LEFT(ImGui::DragFloat3, "Position  ", &emitter.Position.x, 0.1f);
				LABEL_LEFT(ImGui::DragFloat3, "Velocity  ", &emitter.Velocity.x, 0.01f);
				LABEL_LEFT(ImGui::ColorPicker4, "Color     ", &emitter.Color.x);
				float spawnRate = 1.0f / emitter.Lifetime;
				if (LABEL_LEFT(ImGui::DragFloat, "Spawn Rate", &spawnRate, 0.1f, 0.1f)) {
					emitter.Lifetime = 1.0f / spawnRate;
					emitter.Metadata.x = emitter.Lifetime;
				}
				glm::vec2 lifeRange = { emitter.Metadata.z, emitter.Metadata.w };
				if (LABEL_LEFT(ImGui::DragFloat2, "Lifetime  ", &lifeRange.x, 0.1f, 0.0f)) {
					emitter.Metadata.z = lifeRange.x;
					emitter.Metadata.w = lifeRange.y;
				}

				if (ImGuiHelper::WarningButton("Delete")) {
					_emitters.erase(_emitters.begin() + ix);
					ix--;
				}
			}

			ImGui::PopID();
		}

		ImGui::Separator();
		if (ImGui::Button("Add Emitter")) {
			ParticleData emitter;
			emitter.Type = ParticleType::Emitter;
			emitter.Position = glm::vec3(0.0f);
			emitter.Velocity = glm::vec3(0.0f);
			emitter.Color    = glm::vec4(1.0f);
			emitter.Lifetime = 1.0f; 
			emitter.Metadata = { 1.0f, 0.0f, 1.0f, 1.0f };
			_emitters.push_back(emitter);
		}
	}
}

void ParticleSystem::Awake()
{
	// There are the things we want the feedback buffers to track
	const char const* varyings[6] = {
		"out_Type",  
		"out_Position",
		"out_Velocity",
		"out_Color", 
		"out_Lifetime",
		"out_Metadata" 
	}; 

	// This is our transform feedback shader
	_updateShader = ShaderProgram::Create();
	_updateShader->LoadShaderPartFromFile("shaders/vertex_shaders/particles_sim_vs.glsl", ShaderPartType::Vertex);
 	_updateShader->LoadShaderPartFromFile("shaders/geometry_shaders/particle_sim_gs.glsl", ShaderPartType::Geometry);
	_updateShader->RegisterVaryings(varyings, 6, true); // Here we call glTransformFeedbackVaryings, and let it know we want interleaved data
	_updateShader->Link(); 

	// This shader will render the particles
	_renderShader = ShaderProgram::Create();
	_renderShader->LoadShaderPartFromFile("shaders/vertex_shaders/particles_render_vs.glsl", ShaderPartType::Vertex);
	_renderShader->LoadShaderPartFromFile("shaders/fragment_shaders/particles_render_fs.glsl", ShaderPartType::Fragment);
	_renderShader->Link(); 
}

nlohmann::json ParticleSystem::ToJson() const {
	nlohmann::json result = {
		{ "gravity", _gravity },
		{ "max_particles", _maxParticles }
	};

	// Add emitters to the JSON data
	result["emitters"] = nlohmann::json();
	for (const auto& emitter : _emitters) {
		nlohmann::json blob = {
			{ "position", emitter.Position },
			{ "velocity", emitter.Velocity },
			{ "spawn_rate", emitter.Lifetime },
			{ "color", emitter.Color },
			{ "cone_angle", emitter.Metadata.y },
			{ "lifetime_range", glm::vec2(emitter.Metadata.z, emitter.Metadata.w) }
		};
		result["emitters"].push_back(blob);
	}

	return result;
}

ParticleSystem::Sptr ParticleSystem::FromJson(const nlohmann::json& blob) {
	ParticleSystem::Sptr result = std::make_shared<ParticleSystem>();

	result->_gravity = JsonGet(blob, "gravity", result->_gravity);
	result->_maxParticles = JsonGet(blob, "max_particled", result->_maxParticles);

	if (blob.contains("emitters") && blob["emitters"].is_array()) {
		for (const auto& data : blob["emitters"]) {
			ParticleData emitter;
			emitter.Type = ParticleType::Emitter;
			emitter.Position = JsonGet(data, "position", glm::vec3(0.0f));
			emitter.Velocity = JsonGet(data, "velocity", glm::vec3(0.0f));
			emitter.Lifetime = JsonGet(data, "spawn_rate", 1.0f);
			emitter.Color    = JsonGet(data, "color", glm::vec4(1.0f));
			glm::vec2 lifeRange = JsonGet(data, "lifetime_range", glm::vec2(1.0f));
			emitter.Metadata = { emitter.Lifetime, JsonGet(data, "cone_angle", 0.0f), lifeRange.x, lifeRange.y };

			result->_emitters.push_back(emitter);
		}
	}

	return result;
}
