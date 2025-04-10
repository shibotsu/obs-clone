import {
  Carousel,
  CarouselCard,
  CarouselNav,
  CarouselNavButton,
  CarouselNavContainer,
  CarouselViewport,
  CarouselSlider,
  tokens,
  makeStyles,
} from "@fluentui/react-components";
import HomePageStreamItem from "./HomePageStreamItem";
import useResponsiveSlides from "./itemsPerSlide";

const livestreams = [
  {
    id: 1,
    title: "LOS RATONES COMPETITIVE SPRING SPLIT",
    streamer: "Caedrel",
    game: "League of Legends",
    viewers: "28.7K viewers",
    thumbnail:
      "https://i.pcmag.com/imagery/reviews/05i45Hf8jvSS87RYAWXWgeW-4.fit_scale.size_1028x578.v1569474054.jpg",
  },
  {
    id: 2,
    title: "NEW! DROPS 15:00 UTC",
    streamer: "skill4ltu",
    game: "World of Tanks",
    viewers: "2.6K viewers",
    thumbnail:
      "https://mnd-assets.mynewsdesk.com/image/upload/ar_16:9,c_fill,dpr_auto,f_auto,g_xy_center,q_auto:good,w_1782,x_960,y_540/df79edcspiyqkdkd9zq09d",
  },
  {
    id: 3,
    title: "D0cC CFG TODAY",
    streamer: "d0cc_tv",
    game: "Counter-Strike",
    viewers: "935 viewers",
    thumbnail:
      "https://csmarket.gg/blog/wp-content/uploads/2024/02/0cc382629edec933916a6f9912bd0f24-_1_-1536x864.webp",
  },
  {
    id: 4,
    title: "NEW* TOKEN DROPS",
    streamer: "QuickyBaby",
    game: "World of Tanks",
    viewers: "3.7K viewers",
    thumbnail:
      "https://preview.redd.it/dfi4kogtoug71.png?width=1080&crop=smart&auto=webp&s=e4a22e31c6751c6af4000510d17c47f214244f4d",
  },
  {
    id: 5,
    title: "LOS RATONES COMPETITIVE SPRING SPLIT",
    streamer: "Caedrel",
    game: "League of Legends",
    viewers: "28.7K viewers",
    thumbnail:
      "https://i.pcmag.com/imagery/reviews/05i45Hf8jvSS87RYAWXWgeW-4.fit_scale.size_1028x578.v1569474054.jpg",
  },
  {
    id: 6,
    title: "NEW! DROPS 15:00 UTC",
    streamer: "skill4ltu",
    game: "World of Tanks",
    viewers: "2.6K viewers",
    thumbnail:
      "https://mnd-assets.mynewsdesk.com/image/upload/ar_16:9,c_fill,dpr_auto,f_auto,g_xy_center,q_auto:good,w_1782,x_960,y_540/df79edcspiyqkdkd9zq09d",
  },
  {
    id: 7,
    title: "D0cC CFG TODAY",
    streamer: "d0cc_tv",
    game: "Counter-Strike",
    viewers: "935 viewers",
    thumbnail:
      "https://csmarket.gg/blog/wp-content/uploads/2024/02/0cc382629edec933916a6f9912bd0f24-_1_-1536x864.webp",
  },
  {
    id: 8,
    title: "NEW* TOKEN DROPS",
    streamer: "QuickyBaby",
    game: "World of Tanks",
    viewers: "3.7K viewers",
    thumbnail:
      "https://preview.redd.it/dfi4kogtoug71.png?width=1080&crop=smart&auto=webp&s=e4a22e31c6751c6af4000510d17c47f214244f4d",
  },
];

const useClasses = makeStyles({
  container: {
    display: "grid",
    gridTemplateColumns: "1fr",
    gridTemplateRows: "auto 1fr",

    boxShadow: tokens.shadow16,
  },
  card: {
    display: "flex",
    flexDirection: "column",

    padding: "10px",
    minHeight: "100px",
  },
  carousel: {
    flex: 1,
    padding: "5px",
  },
  livestreamGroup: {
    display: "flex",
    gap: "16px",
    padding: "10px",
    justifyContent: "center",
    flex: "0 0 100%", // this makes each slide take full width
    boxSizing: "border-box",
  },
  controls: {
    display: "flex",
    flexDirection: "column",
    gap: "6px",

    border: `${tokens.strokeWidthThicker} solid ${tokens.colorNeutralForeground3}`,
    borderBottom: "none",
    borderRadius: tokens.borderRadiusMedium,
    borderBottomLeftRadius: 0,
    borderBottomRightRadius: 0,

    padding: "10px",
  },
  field: {
    flex: 1,
    gridTemplateColumns: "minmax(100px, max-content) 1fr",
  },
  dropdown: {
    maxWidth: "max-content",
  },
});

function chunkArray(array, size) {
  const result = [];
  for (let i = 0; i < array.length; i += size) {
    result.push(array.slice(i, i + size));
  }
  return result;
}

const HomeLiveStreams = () => {
  const classes = useClasses();
  const itemsPerSlide = useResponsiveSlides();
  const slides = chunkArray(livestreams, itemsPerSlide);

  return (
    <div>
      <Carousel
        align="start"
        className={classes.carousel}
        whitespace={true}
        announcement={(index) =>
          `Carousel slide ${index + 1} of ${slides.length}`
        }
      >
        <CarouselViewport>
          <CarouselSlider cardFocus>
            {slides.map((group, slideIndex) => (
              <CarouselCard key={slideIndex}>
                <div className={classes.livestreamGroup}>
                  {group.map((livestream) => (
                    <HomePageStreamItem
                      key={livestream.id}
                      id={livestream.id}
                      title={livestream.title}
                      streamer={livestream.streamer}
                      game={livestream.game}
                      viewers={livestream.viewers}
                      thumbnail={livestream.thumbnail}
                    />
                  ))}
                </div>
              </CarouselCard>
            ))}
          </CarouselSlider>
        </CarouselViewport>
        <CarouselNavContainer
          layout="inline"
          next={{ "aria-label": "go to next" }}
          prev={{ "aria-label": "go to prev" }}
        >
          <CarouselNav>
            {(index) => (
              <CarouselNavButton aria-label={`Carousel Nav Button ${index}`} />
            )}
          </CarouselNav>
        </CarouselNavContainer>
      </Carousel>
    </div>
  );
};

export default HomeLiveStreams;
