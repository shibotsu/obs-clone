import { useNavigate } from "react-router-dom";
import { Text } from "@fluentui/react";
import { useQuery } from "@tanstack/react-query";
import { Skeleton } from "@fluentui/react-components";
import HomePageStreamerItem from "../../streamer/HomePageStreamerItem";
import "./MostFollowedStreamers.css";

const MostFollowedStreamers = () => {
  let navigate = useNavigate();

  const { data, isLoading } = useQuery({
    queryFn: () =>
      fetch("http://127.0.0.1:8000/api/most_followed", {
        method: "GET",
        headers: {
          "Content-Type": "application/json",
        },
      }).then((res) => res.json()),
    queryKey: ["mostFollowedStreamers"],
  });

  return (
    <div className="most-followed-container">
      <Text variant="xxLarge" className="most-followed">
        Most followed streamers
      </Text>
      <div className="most-followed-streamers">
        {isLoading ? (
          <Skeleton />
        ) : (
          data?.users?.map((user) => {
            const isNone =
              user.profile_picture === "none" ||
              user.profile_picture.endsWith("/none");

            const profilePicture = isNone
              ? "profile_pic_placeholder.png"
              : user.profile_picture;

            return (
              <div key={user.id}>
                <HomePageStreamerItem
                  username={user.username}
                  profilePic={profilePicture}
                  onClick={() => navigate(`/streamer-profile/${user.id}`)}
                />
              </div>
            );
          })
        )}
      </div>
    </div>
  );
};

export default MostFollowedStreamers;
