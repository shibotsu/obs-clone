import {
  Carousel,
  CarouselCard,
  CarouselNav,
  CarouselNavButton,
  CarouselNavContainer,
  CarouselViewport,
  CarouselSlider,
  Text,
  tokens,
  makeStyles,
} from "@fluentui/react-components";
import HomePageStreamItem from "./HomePageStreamItem";
import useResponsiveSlides from "./itemsPerSlide";
import { useQuery } from "@tanstack/react-query";
import { useAuth } from "../../../context/AuthContext";

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
  emptyState: {
    display: "flex",
    justifyContent: "center",
    alignItems: "center",
    height: "150px", // adjust height as needed
    textAlign: "center",
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

  const { data, isLoading } = useQuery({
    queryKey: ["homePageLivestreams"],
    queryFn: async () => {
      const response = await fetch("http://157.230.16.67:8000/api/channels");

      if (!response.ok) {
        return new Error("Unable to load livestreams.");
      }

      return response.json();
    },
  });

  console.log(data);

  const channels = Array.isArray(data?.channels) ? data?.channels : [];
  const slides = chunkArray(channels, itemsPerSlide);

  return (
    <div>
      {channels.length === 0 ? (
        <Text size={600} className={classes.emptyState}>
          No livestreams available currently.
        </Text>
      ) : (
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
                    {group.map((channels) => (
                      <HomePageStreamItem
                        key={channels.id}
                        id={channels.user_id}
                        title={channels.title}
                        streamer={channels.streamer_name}
                        game={channels.stream_category}
                        viewers={0}
                        thumbnail={channels.thumbnail}
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
                <CarouselNavButton
                  aria-label={`Carousel Nav Button ${index}`}
                />
              )}
            </CarouselNav>
          </CarouselNavContainer>
        </Carousel>
      )}
    </div>
  );
};

export default HomeLiveStreams;
