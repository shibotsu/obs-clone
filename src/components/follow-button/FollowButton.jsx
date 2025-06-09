import { useQuery, useMutation, useQueryClient } from "@tanstack/react-query";
import { useAuth } from "../../context/AuthContext";
import { useState } from "react";
import { Button, Spinner } from "@fluentui/react-components";

export const FollowButton = ({ id }) => {
  const { token } = useAuth();
  const queryClient = useQueryClient();
  const [pending, setPending] = useState(false);

  const { data, loading, error, isLoading } = useQuery({
    queryKey: ["followStatus", id],
    queryFn: () =>
      fetch(`http://157.230.16.67:8000/api/isfollowing/${id}`, {
        method: "GET",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`,
        },
      }).then((res) => res.json()),
  });

  const followMutation = useMutation({
    mutationFn: () =>
      fetch(`http://157.230.16.67:8000/api/follow/${id}`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`,
        },
      }),

    onSuccess: () => {
      queryClient.invalidateQueries(["followStatus", id]);
    },
  });

  const unfollowMutation = useMutation({
    mutationFn: () =>
      fetch(`http://157.230.16.67:8000/api/unfollow/${id}`, {
        method: "DELETE",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`,
        },
      }),

    onSuccess: () => {
      queryClient.invalidateQueries(["followStatus", id]);
    },
  });

  const isMutating = unfollowMutation.isPending || followMutation.isPending;

  const isFollowing = data?.isFollowing;

  const handleClick = async () => {
    setPending(true);
    if (isFollowing) {
      unfollowMutation.mutateAsync();
    } else {
      followMutation.mutateAsync();
    }
    setPending(false);
  };

  const buttonIcon =
    isMutating || loading || isLoading || pending ? (
      <Spinner size="tiny" />
    ) : null;

  return (
    <div>
      {token && (
        <Button
          disabled={isMutating || isLoading || pending}
          onClick={handleClick}
          icon={buttonIcon}
          appearance={isFollowing ? "default" : "primary"}
        >
          {isFollowing ? "Unfollow" : "Follow"}
        </Button>
      )}
    </div>
  );
};
