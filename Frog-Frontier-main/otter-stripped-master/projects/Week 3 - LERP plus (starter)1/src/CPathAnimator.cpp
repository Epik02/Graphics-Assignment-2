/*
NOU Framework - Created for INFR 2310 at Ontario Tech.
(c) Samantha Stahlke 2020

CPathAnimator.cpp
Simple component for animating an object along a path.

As a convention in NOU, we put "C" before a class name to signify
that we intend the class for use as a component with the ENTT framework.
*/

#include "CPathAnimator.h"
#include "iostream"
namespace nou
{
	CPathAnimator::CPathAnimator(Entity& owner)
	{
		m_owner = &owner;
		m_index1 = 0;
		m_index2 = 1;
		m_segmentIndex = 0;
		m_segmentTimer = 0.0f;

		m_segmentTravelTime = 0.1f;
		m_mode = PathSampler::PathMode::LERP;
	}

	void CPathAnimator::SetMode(PathSampler::PathMode mode)
	{
		m_mode = mode;
		m_segmentIndex = 0;
	}
	
	void CPathAnimator::Update(const PathSampler::KeypointSet& keypoints, float deltaTime)
	{
		std::cout << m_segmentTimer << std::endl;

		if (keypoints.size() > 1) {
			if (increase == true)
			{
				m_segmentTimer += 0.1;
			}
			else if (increase == false)
			{
				m_segmentTimer -= 0.1;
			}

			if (m_segmentTimer >= 1)
			{
				increase = false;
			}
			else if (m_segmentTimer <= 0.1)
			{
				increase = true;
			}
			m_owner->transform.m_pos.x = PathSampler::LERP(keypoints[m_index1]->transform.m_pos.x, keypoints[m_index2]->transform.m_pos.x, m_segmentTimer);
			m_owner->transform.m_pos.y = PathSampler::LERP(keypoints[m_index1]->transform.m_pos.y, keypoints[m_index2]->transform.m_pos.y, m_segmentTimer);
			m_owner->transform.m_pos.z = PathSampler::LERP(keypoints[m_index1]->transform.m_pos.z, keypoints[m_index2]->transform.m_pos.z, m_segmentTimer);
		}
		
		// TODO: Implement this function
		// Steps to follow:

		// Make sure we have points to use

		// Increment our t value

		// Handle switching segments each time t = 1

		// Work out t using segment timer and segment travel time

		// Ensure we have at least 2 points to LERP along
		// Need at least 2 points for 1 segment

		// Determine the indices of the points to LERP along (in our vector of key points)

		// Perform LERP using function in PathSampler
	}
}