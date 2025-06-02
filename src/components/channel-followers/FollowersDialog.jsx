import { useState } from "react";
import {
  Dialog,
  DialogTrigger,
  DialogSurface,
  DialogTitle,
  DialogContent,
  DialogBody,
  DialogActions,
  Button,
  Text,
  Spinner,
} from "@fluentui/react-components";
import { useQuery } from "@tanstack/react-query";
import { useAuth } from "../../context/AuthContext";
import { useNavigate } from "react-router-dom";
import FollowingSidebarItem from "../sidebar/FollowingSidebarItem";

const FollowersDialog = ({ followers, userId }) => {
  const [isOpen, setIsOpen] = useState(false);
  const { token } = useAuth();
  let navigate = useNavigate();

  const {
    data: followersObject,
    isLoading,
    error,
  } = useQuery({
    queryKey: ["followers", userId],
    queryFn: async () => {
      const response = await fetch(
        `http://127.0.0.1:8000/api/${userId}/followers`,
        {
          headers: {
            Authorization: `Bearer ${token}`,
          },
        }
      );

      if (!response.ok) {
        throw new Error("Failed to fetch followers");
      }

      const data = await response.json();
      return data;
    },
    enabled: isOpen, // Only fetch when dialog is opened
  });

  const followersList = followersObject?.followers || [];

  return (
    <div>
      <Dialog
        open={isOpen}
        onOpenChange={(event, data) => setIsOpen(data.open)}
      >
        <DialogTrigger disableButtonEnhancement>
          <Text
            size={700}
            className="follower-count"
            style={{ cursor: "pointer" }}
          >
            Followers: {followers}
          </Text>
        </DialogTrigger>

        <DialogSurface style={{ minWidth: "400px", maxWidth: "600px" }}>
          <DialogBody>
            <DialogTitle>Followers ({followers})</DialogTitle>
            <DialogContent>
              {isLoading && (
                <div
                  style={{
                    display: "flex",
                    justifyContent: "center",
                    padding: "20px",
                  }}
                >
                  <Spinner label="Loading followers..." />
                </div>
              )}

              {error && (
                <Text style={{ color: "red", padding: "20px" }}>
                  Error loading followers. Please try again.
                </Text>
              )}

              {followersList && (
                <div style={{ maxHeight: "400px", overflowY: "auto" }}>
                  {followersList.map((user) => (
                    <FollowingSidebarItem
                      key={user.id}
                      id={user.id}
                      name={user.username}
                      avatar={user.profile_picture}
                      onClick={() => navigate(`/streamer-profile/${user.id}`)}
                    />
                  ))}
                </div>
              )}
            </DialogContent>

            <DialogActions>
              <Button appearance="secondary" onClick={() => setIsOpen(false)}>
                Close
              </Button>
            </DialogActions>
          </DialogBody>
        </DialogSurface>
      </Dialog>
    </div>
  );
};

export default FollowersDialog;
