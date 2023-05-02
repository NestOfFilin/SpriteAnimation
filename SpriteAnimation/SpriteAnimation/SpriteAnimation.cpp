#include "SpriteAnimation.h"


CSpritesCountGerenator::CSpritesCountGerenator(const sf::Image& image)
	: m_Image(image)
{
}

CSpritesCountGerenator::~CSpritesCountGerenator() = default;


CSimpleSCGerenator::CSimpleSCGerenator(const sf::Image& image)
	: CSpritesCountGerenator(image)
{
}


CColorColumnSCGerenator::CColorColumnSCGerenator(
	const sf::Image& image,
	const sf::Color separator/* = sf::Color(0U, 0U, 0U, 0U)*/)
	: CSpritesCountGerenator(image)
	, m_Separator(separator)
{
}

unsigned int CColorColumnSCGerenator::Generate()
{
	unsigned int spritesCount = 0U;
	const auto& image = GetImage();
	const sf::Vector2u imageSize = image.getSize();

	bool isPrevColumnSeparator = true;
	for (unsigned int x = 0; x < imageSize.x; x++)
	{
		bool columnNotSeparator = false;
		for (unsigned int y = 0; y < imageSize.y; y++)
		{
			if (image.getPixel(x, y) != m_Separator)
			{
				columnNotSeparator = true;
				break;
			}
		}

		if (columnNotSeparator && isPrevColumnSeparator)
		{
			spritesCount++;
			isPrevColumnSeparator = false;
		}
		else if (!columnNotSeparator)
		{
			isPrevColumnSeparator = true;
		}
	}

	if (spritesCount == 0U)
	{
		spritesCount++;
	}

	return spritesCount;
}


std::unique_ptr<CSpritesCountGerenator> CSCGerenatorFactory::MakeGenerator(
	const sf::Image& spriteTextureImage,
	const ESCGerenatorType scGenType)
{
	switch (scGenType)
	{
	case ESCGerenatorType::Simple:
		return std::make_unique<CSimpleSCGerenator>(spriteTextureImage);
	case ESCGerenatorType::ColorColumn:
		return std::make_unique<CColorColumnSCGerenator>(spriteTextureImage);
	default:
		throw L"Unknown SpritesCountGerenator type.";
		return nullptr;
	}
}


CSpriteAnimation::CSpriteAnimation(
	const sf::Image& spriteTextureImage,
	const ESCGerenatorType scGenType/* = ESCGerenatorType::Simple*/)
	: m_SpriteTexture(
		[](const sf::Image& spriteTextureImage) -> sf::Texture
		{
			sf::Texture spriteTexture;
			if (!spriteTexture.loadFromImage(spriteTextureImage))
			{
				throw EXIT_FAILURE;
			}
			return spriteTexture;
		}(spriteTextureImage))
	, m_Sprite(m_SpriteTexture)
	, m_SpritesCount(
		[](const sf::Image& spriteTextureImage,
			const ESCGerenatorType scGenType
		) -> unsigned int
		{
			auto generator = CSCGerenatorFactory::MakeGenerator(
				spriteTextureImage,
				scGenType
			);
			return generator->Generate();
		}(spriteTextureImage, scGenType))
{
	const auto& textureRect = m_Sprite.getTextureRect();
	m_Sprite.setTextureRect(sf::IntRect(
		textureRect.left,
		textureRect.top,
		textureRect.width / m_SpritesCount,
		textureRect.height
	));
}

void CSpriteAnimation::PutNextSprite()
{
	m_CurrentSpriteIndex++;
	if (m_CurrentSpriteIndex == m_SpritesCount)
	{
		m_CurrentSpriteIndex = 0U;
	}

	const auto& textureRect = m_Sprite.getTextureRect();
	m_Sprite.setTextureRect(
		sf::IntRect(
			textureRect.width * m_CurrentSpriteIndex,
			textureRect.top,
			textureRect.width,
			textureRect.height)
	);
}
