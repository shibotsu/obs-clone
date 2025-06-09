import { makeStyles, Text } from "@fluentui/react-components";
import { useLocation } from "react-router-dom";
import { useQuery } from "@tanstack/react-query";
import { useEffect } from "react";
import { Spinner } from "@fluentui/react-components";
import FollowingSidebarItem from "../../components/sidebar/FollowingSidebarItem";

const useStyles = makeStyles({
  pageContainer: {
    width: "90%",
    maxWidth: "1200px",
    margin: "20px auto", // center the page with vertical margin
    padding: "20px",
    fontFamily: "Segoe UI, sans-serif",
  },
  header: {
    textAlign: "center",
    marginBottom: "30px",
    fontWeight: "600",
  },
  resultsContainer: {
    display: "grid",
    gridTemplateColumns: "repeat(auto-fit, minmax(250px, 1fr))",
    gap: "20px",
  },
  noResults: {
    position: "absolute",
    left: "50%",
    top: "50%",
    textAlign: "center",
    marginTop: "20px",
    color: "#555",
    fontStyle: "italic",
  },
});

const SearchResultsPage = () => {
  const classes = useStyles();
  const { state } = useLocation();

  // We assume that the search results have been passed via the state.
  const initialResults = state?.results || [];
  const queryToUse = state?.query || "";

  console.log(JSON.stringify(state));

  const { data, refetch, isLoading } = useQuery({
    queryKey: ["searchResults", queryToUse],
    queryFn: () => {
      fetch("http://157.230.16.67:8000/api/search", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ query: queryToUse }),
      }).then((res) => res.json());
    },
    enabled: queryToUse.trim().length > 0,
    initialData:
      initialResults.length > 0 ? { users: initialResults } : undefined,
  });

  // If fetched data is empty and not loading, wait and refetch
  useEffect(() => {
    if (!isLoading && data && data.users && data.users.length === 0) {
      const timeout = setTimeout(() => {
        refetch();
      }, 2000);
      return () => clearTimeout(timeout);
    }
  }, [data, isLoading, refetch]);

  const users = data ? data.users : [];

  return (
    <div className={classes.pageContainer}>
      <Text variant="xxLarge" className={classes.header}>
        Search Results for "{queryToUse}"
      </Text>
      {isLoading ? (
        <Spinner style={{ position: "absolute", left: "50%", top: "50%" }} />
      ) : users.length > 0 ? (
        <div className={classes.resultsContainer}>
          {users.map((user) => (
            <FollowingSidebarItem
              key={user.id}
              id={user.id}
              avatar={user.profile_picture}
              name={user.username}
            />
          ))}
        </div>
      ) : (
        <Text variant="medium" className={classes.noResults}>
          No results found.
        </Text>
      )}
    </div>
  );
};

export default SearchResultsPage;
