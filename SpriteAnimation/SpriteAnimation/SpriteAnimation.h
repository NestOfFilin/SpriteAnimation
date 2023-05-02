#pragma once
#include <SFML/Graphics.hpp>
#include <memory>


enum class ESCGerenatorType
{
	Simple = 1,
	ColorColumn = 2
};


class CSpritesCountGerenator
{
public:
	CSpritesCountGerenator() = delete;
	CSpritesCountGerenator(const sf::Image& image);

	virtual ~CSpritesCountGerenator() = 0;

	virtual unsigned int Generate() = 0;

	virtual ESCGerenatorType GetType() = 0;

	inline const sf::Image& GetImage()
	{
		return m_Image;
	}

private:
	const sf::Image& m_Image;
};


class CSimpleSCGerenator final
	: public CSpritesCountGerenator
{
public:
	CSimpleSCGerenator(const sf::Image& image);

	~CSimpleSCGerenator() = default;

	inline unsigned int Generate() override
	{
		return 1U;
	}

	inline ESCGerenatorType GetType() override
	{
		return ESCGerenatorType::Simple;
	}
};


class CColorColumnSCGerenator final
	: public CSpritesCountGerenator
{
public:
	CColorColumnSCGerenator(
		const sf::Image& image,
		const sf::Color separator = sf::Color(0U, 0U, 0U, 0U)
	);

	~CColorColumnSCGerenator() = default;

	unsigned int Generate() override;

	inline ESCGerenatorType GetType() override
	{
		return ESCGerenatorType::ColorColumn;
	}

private:
	const sf::Color m_Separator;
};


class CSCGerenatorFactory
{
public:
	static std::unique_ptr<CSpritesCountGerenator> MakeGenerator(
		const sf::Image& spriteTextureImage,
		const ESCGerenatorType scGenType
	);

private:
	CSCGerenatorFactory() = default;
	~CSCGerenatorFactory() = default;
};


class CSpriteAnimation
{
public:
	CSpriteAnimation(
		const sf::Image& spriteTextureImage,
		const ESCGerenatorType scGenType = ESCGerenatorType::Simple
	);

	inline sf::Sprite& GetSprite()
	{
		return m_Sprite;
	}

	void PutNextSprite();

private:
	sf::Texture m_SpriteTexture;
	sf::Sprite m_Sprite;

	unsigned int m_CurrentSpriteIndex{ 0U };
	unsigned int m_SpritesCount;
};