import { useState } from "react";
import { FollowButton } from "../../components/follow-button/FollowButton";
import { useQuery } from "@tanstack/react-query";
import { useParams } from "react-router-dom";
import { Image, Spinner } from "@fluentui/react-components";
import { Text, Stack } from "@fluentui/react";
import { useAuth } from "../../context/AuthContext";
import "./Profile.css";

const StreamerProfilePage = () => {
  const { id } = useParams();
  const { data, isLoading, error } = useQuery({
    queryFn: () =>
      fetch(`http://127.0.0.1:8000/api/profile/${id}`, {
        headers: {
          "Content-Type": "application/json",
        },
      }).then((res) => res.json()),
    queryKey: ["streamerProfile", id],
  });

  if (isLoading) {
    return <Spinner style={{ marginTop: "10rem" }} />;
  }

  if (error || !data || !data.channel) {
    return <div>An error occured.</div>;
  }

  const { username, profile_picture, number_of_followers } = data.channel;
  const profilePic =
    profile_picture?.endsWith("/none") || profile_picture === "none"
      ? "/profile_pic_placeholder.png"
      : profile_picture;

  return (
    <div className="profile-container">
      {console.log(JSON.stringify(data))}
      {console.log("Profile pic: " + profilePic)}
      <Stack tokens={{ childrenGap: 20 }} className="profile-card">
        <div className="profile-pic-container">
          <Image
            src={profilePic}
            shape="circular"
            alt="Profile picture"
            width={150}
            height={150}
            className="profile-pic"
          />
        </div>
        <Text variant="xxLarge" className="profile-username">
          {username}
        </Text>
        <Text variant="medium" className="follower-count">
          Followers: {number_of_followers ?? 0}
        </Text>
        <FollowButton id={id} />
      </Stack>
    </div>
  );
};

export default StreamerProfilePage;
